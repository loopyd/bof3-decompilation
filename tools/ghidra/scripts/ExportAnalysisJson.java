// Export all Ghidra project analysis data to a single JSON consumed by
// bin/build-db.  Replaces ExportSymbolsJson + ExportDuplicateGroups.
//
// Usage (headless):
//   analyzeHeadless <project-dir> <project-name> -process / -recursive \
//     -scriptPath <this-dir> \
//     -postScript ExportAnalysisJson.java <output-json> [/project-path]
//
// Output sections: functions, symbols, xrefs, call_edges, constants, duplicates
//
// Output schema: harness.ghidra-analysis-export/v1

import ghidra.app.script.GhidraScript;
import ghidra.framework.model.DomainFile;
import ghidra.framework.model.DomainFolder;
import ghidra.framework.model.ProjectData;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSetView;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.mem.MemoryAccessException;
import ghidra.program.model.symbol.*;

import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.*;

public class ExportAnalysisJson extends GhidraScript {

    /* ---- data structures ---- */

    private static final class FuncRec {
        String programPath;
        String address;
        String name;
        String signature;
        String bodyMin;
        String bodyMax;
        String namespace;
        String nameSource;
        boolean thunk;
    }

    private static final class SymRec {
        String programPath;
        String address;
        String name;
        String kind;
        String nameSource;
    }

    private static final class XrefRec {
        String programPath;
        String fromAddress;
        String toAddress;
        String referenceType;
    }

    private static final class CallEdge {
        String fromFunc;
        String toFunc;
        String fromProgram;
        boolean toExternal;
    }

    private static final class ConstRec {
        String programPath;
        String address;
        String name;
        String dataType;
        int xrefCount;
    }

    private static final class DupEntry {
        String programPath;
        String address;
        String name;
        String bodyMin;
        String bodyMax;
        String nameSource;
        String signature;
        int size;
    }

    /* ---- main ---- */

    @Override
    protected void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 1) {
            throw new IllegalArgumentException(
                "usage: ExportAnalysisJson.java <output-json> [project-path]");
        }

        ProjectData projectData = state.getProject().getProjectData();
        String selectedPath = args.length > 1 ? args[1] : "/";

        List<FuncRec> functions = new ArrayList<>();
        List<SymRec> symbols = new ArrayList<>();
        List<XrefRec> xrefs = new ArrayList<>();
        List<CallEdge> callEdges = new ArrayList<>();
        List<ConstRec> constants = new ArrayList<>();
        Map<String, List<DupEntry>> dupGroups = new HashMap<>();

        collectAll(projectData.getRootFolder(), selectedPath,
                   functions, symbols, xrefs, callEdges, constants, dupGroups);

        Path outputPath = Paths.get(args[0]);
        Path parent = outputPath.getParent();
        if (parent != null) {
            Files.createDirectories(parent);
        }
        try (PrintWriter w = new PrintWriter(
                Files.newBufferedWriter(outputPath, StandardCharsets.UTF_8))) {
            writeJson(w, functions, symbols, xrefs, callEdges, constants, dupGroups);
        }

        printf("Exported: %d functions, %d symbols, %d xrefs, %d call_edges, %d constants, %d dup_groups\n",
               functions.size(), symbols.size(), xrefs.size(),
               callEdges.size(), constants.size(), countMultiProgramGroups(dupGroups));
    }

    /* ---- collection ---- */

    private void collectAll(DomainFolder folder, String selectedPath,
                            List<FuncRec> functions, List<SymRec> symbols,
                            List<XrefRec> xrefs, List<CallEdge> callEdges,
                            List<ConstRec> constants,
                            Map<String, List<DupEntry>> dupGroups) throws Exception {
        monitor.checkCancelled();
        for (DomainFile file : folder.getFiles()) {
            monitor.checkCancelled();
            if (Program.class.isAssignableFrom(file.getDomainObjectClass())
                    && isSelected(file.getPathname(), selectedPath)) {
                collectProgram(file, functions, symbols, xrefs, callEdges,
                               constants, dupGroups);
            }
        }
        for (DomainFolder child : folder.getFolders()) {
            collectAll(child, selectedPath, functions, symbols, xrefs, callEdges,
                       constants, dupGroups);
        }
    }

    private boolean isSelected(String path, String selected) {
        if (selected == null || selected.trim().length() == 0 || selected.equals("/")) {
            return true;
        }
        String norm = selected.startsWith("/") ? selected : "/" + selected;
        return path.equals(norm) || path.startsWith(norm + "/");
    }

    private void collectProgram(DomainFile file,
                                List<FuncRec> functions, List<SymRec> symbols,
                                List<XrefRec> xrefs, List<CallEdge> callEdges,
                                List<ConstRec> constants,
                                Map<String, List<DupEntry>> dupGroups) throws Exception {
        Program program = null;
        try {
            program = (Program) file.getDomainObject(this, false, false, monitor);
            String programPath = file.getPathname();
            Memory memory = program.getMemory();
            FunctionManager fm = program.getFunctionManager();

            // track data symbol xref counts for constants
            Map<Address, Integer> dataXrefCounts = collectDataXrefCounts(program);

            for (Function func : fm.getFunctions(true)) {
                monitor.checkCancelled();
                SourceType source = func.getSymbol().getSource();
                boolean isImported = source == SourceType.IMPORTED;
                boolean isThunk = func.isThunk();

                // --- functions ---
                FuncRec fr = new FuncRec();
                fr.programPath = programPath;
                fr.address = func.getEntryPoint().toString();
                fr.name = func.getName();
                fr.signature = func.getSignature().getPrototypeString();
                AddressSetView body = func.getBody();
                fr.bodyMin = body == null ? null : body.getMinAddress().toString();
                fr.bodyMax = body == null ? null : body.getMaxAddress().toString();
                fr.namespace = func.getParentNamespace().getName(true);
                fr.nameSource = source.name();
                fr.thunk = isThunk;
                functions.add(fr);

                // --- call edges ---
                Set<Function> called = func.getCalledFunctions(monitor);
                for (Function callee : called) {
                    CallEdge edge = new CallEdge();
                    edge.fromFunc = func.getEntryPoint().toString();
                    edge.toFunc = callee.getEntryPoint().toString();
                    edge.fromProgram = programPath;
                    edge.toExternal = callee.isExternal();
                    callEdges.add(edge);
                }

                // --- duplicates (skip imported/thunks, limit RAM range) ---
                if (!isImported && !isThunk
                        && func.getEntryPoint().getOffset() >= 0x80000000L) {
                    long bodyStart = func.getBody().getMinAddress().getOffset();
                    long bodyEnd = func.getBody().getMaxAddress().getOffset();
                    int size = (int)(bodyEnd - bodyStart + 1);
                    if (size > 0 && size <= 65536) {
                        byte[] bytes = new byte[size];
                        try {
                            memory.getBytes(func.getBody().getMinAddress(), bytes);
                            String sha = sha256Hex(bytes);
                            DupEntry de = new DupEntry();
                            de.programPath = programPath;
                            de.address = func.getEntryPoint().toString();
                            de.name = func.getName();
                            de.bodyMin = func.getBody().getMinAddress().toString();
                            de.bodyMax = func.getBody().getMaxAddress().toString();
                            de.nameSource = source.name();
                            de.signature = func.getSignature().getPrototypeString();
                            de.size = size;
                            List<DupEntry> group = dupGroups.get(sha);
                            if (group == null) {
                                group = new ArrayList<>();
                                dupGroups.put(sha, group);
                            }
                            group.add(de);
                        } catch (MemoryAccessException e) {
                            // skip
                        }
                    }
                }
            }

            // --- symbols (non-function) ---
            for (Symbol sym : program.getSymbolTable().getAllSymbols(true)) {
                if (sym.getSymbolType().toString().equals("Function")) {
                    continue;
                }
                SymRec sr = new SymRec();
                sr.programPath = programPath;
                sr.address = sym.getAddress().toString();
                sr.name = sym.getName();
                sr.kind = sym.getSymbolType().toString();
                SourceType s = sym.getSource();
                sr.nameSource = s == null ? null : s.name();
                symbols.add(sr);
            }

            // --- xrefs ---
            ReferenceIterator refs = program.getReferenceManager()
                .getReferenceIterator(program.getMemory().getMinAddress());
            while (refs.hasNext()) {
                Reference ref = refs.next();
                XrefRec xr = new XrefRec();
                xr.programPath = programPath;
                xr.fromAddress = ref.getFromAddress().toString();
                xr.toAddress = ref.getToAddress().toString();
                xr.referenceType = ref.getReferenceType().getName();
                xrefs.add(xr);
            }

            // --- constants (data symbols with xrefs >= 2) ---
            for (Symbol sym : program.getSymbolTable().getAllSymbols(true)) {
                if (sym.getSymbolType().toString().equals("Function")) {
                    continue;
                }
                if (sym.getSource() != SourceType.DEFAULT
                        && sym.getSource() != SourceType.USER_DEFINED) {
                    continue;
                }
                // only address labels that look like data references
                String name = sym.getName();
                if (name == null || !name.startsWith("D_")) {
                    continue;
                }
                Address addr = sym.getAddress();
                int count = dataXrefCounts.getOrDefault(addr, 0);
                if (count < 2) {
                    continue;
                }
                ConstRec cr = new ConstRec();
                cr.programPath = programPath;
                cr.address = addr.toString();
                cr.name = name;
                Data data = program.getListing().getDataAt(addr);
                cr.dataType = data == null ? null : data.getDataType().getDisplayName();
                cr.xrefCount = count;
                constants.add(cr);
            }
        } finally {
            if (program != null) {
                program.release(this);
            }
        }
    }

    private Map<Address, Integer> collectDataXrefCounts(Program program) {
        Map<Address, Integer> counts = new HashMap<>();
        ReferenceManager rm = program.getReferenceManager();
        SymbolTable st = program.getSymbolTable();
        for (Symbol sym : st.getAllSymbols(true)) {
            if (sym.getSymbolType().toString().equals("Function")) {
                continue;
            }
            ReferenceIterator refs = rm.getReferencesTo(sym.getAddress());
            int c = 0;
            while (refs.hasNext() && c < 10000) {
                refs.next();
                c++;
            }
            counts.put(sym.getAddress(), c);
        }
        return counts;
    }

    /* ---- JSON output ---- */

    private void writeJson(PrintWriter w,
                           List<FuncRec> functions, List<SymRec> symbols,
                           List<XrefRec> xrefs, List<CallEdge> callEdges,
                           List<ConstRec> constants,
                           Map<String, List<DupEntry>> dupGroups) {
        w.println("{");
        w.println("  \"schema\": \"harness.ghidra-analysis-export/v1\",");
        w.println("  \"project_name\": " + js(state.getProject().getName()) + ",");

        writeFunctions(w, functions);
        w.println(",");

        writeSymbols(w, symbols);
        w.println(",");

        writeXrefs(w, xrefs);
        w.println(",");

        writeCallEdges(w, callEdges);
        w.println(",");

        writeConstants(w, constants);
        w.println(",");

        writeDuplicates(w, dupGroups);

        w.println();
        w.println("}");
    }

    private void writeFunctions(PrintWriter w, List<FuncRec> list) {
        Collections.sort(list, new Comparator<FuncRec>() {
            public int compare(FuncRec a, FuncRec b) {
                int c = compareText(a.programPath, b.programPath);
                return c != 0 ? c : compareText(a.address, b.address);
            }
        });
        w.println("  \"functions\": [");
        for (int i = 0; i < list.size(); i++) {
            FuncRec r = list.get(i);
            w.println("    {");
            w.println("      \"program_path\": " + js(r.programPath) + ",");
            w.println("      \"address\": " + js(r.address) + ",");
            w.println("      \"name\": " + js(r.name) + ",");
            w.println("      \"signature\": " + js(r.signature) + ",");
            w.println("      \"body_min\": " + js(r.bodyMin) + ",");
            w.println("      \"body_max\": " + js(r.bodyMax) + ",");
            w.println("      \"namespace\": " + js(r.namespace) + ",");
            w.println("      \"name_source\": " + js(r.nameSource) + ",");
            w.println("      \"is_thunk\": " + r.thunk);
            w.print("    }");
            w.println(i + 1 == list.size() ? "" : ",");
        }
        w.println("  ]");
    }

    private void writeSymbols(PrintWriter w, List<SymRec> list) {
        Collections.sort(list, new Comparator<SymRec>() {
            public int compare(SymRec a, SymRec b) {
                int c = compareText(a.programPath, b.programPath);
                return c != 0 ? c : compareText(a.address, b.address);
            }
        });
        w.println("  \"symbols\": [");
        for (int i = 0; i < list.size(); i++) {
            SymRec r = list.get(i);
            w.println("    {");
            w.println("      \"program_path\": " + js(r.programPath) + ",");
            w.println("      \"address\": " + js(r.address) + ",");
            w.println("      \"name\": " + js(r.name) + ",");
            w.println("      \"kind\": " + js(r.kind) + ",");
            w.println("      \"name_source\": " + js(r.nameSource));
            w.print("    }");
            w.println(i + 1 == list.size() ? "" : ",");
        }
        w.println("  ]");
    }

    private void writeXrefs(PrintWriter w, List<XrefRec> list) {
        Collections.sort(list, new Comparator<XrefRec>() {
            public int compare(XrefRec a, XrefRec b) {
                int c = compareText(a.programPath, b.programPath);
                if (c != 0) return c;
                c = compareText(a.fromAddress, b.fromAddress);
                return c != 0 ? c : compareText(a.toAddress, b.toAddress);
            }
        });
        w.println("  \"xrefs\": [");
        for (int i = 0; i < list.size(); i++) {
            XrefRec r = list.get(i);
            w.println("    {");
            w.println("      \"program_path\": " + js(r.programPath) + ",");
            w.println("      \"from_address\": " + js(r.fromAddress) + ",");
            w.println("      \"to_address\": " + js(r.toAddress) + ",");
            w.println("      \"reference_type\": " + js(r.referenceType));
            w.print("    }");
            w.println(i + 1 == list.size() ? "" : ",");
        }
        w.println("  ]");
    }

    private void writeCallEdges(PrintWriter w, List<CallEdge> list) {
        Collections.sort(list, new Comparator<CallEdge>() {
            public int compare(CallEdge a, CallEdge b) {
                int c = compareText(a.fromProgram, b.fromProgram);
                if (c != 0) return c;
                c = compareText(a.fromFunc, b.fromFunc);
                return c != 0 ? c : compareText(a.toFunc, b.toFunc);
            }
        });
        w.println("  \"call_edges\": [");
        for (int i = 0; i < list.size(); i++) {
            CallEdge r = list.get(i);
            w.println("    {");
            w.println("      \"from_func\": " + js(r.fromFunc) + ",");
            w.println("      \"to_func\": " + js(r.toFunc) + ",");
            w.println("      \"from_program\": " + js(r.fromProgram) + ",");
            w.println("      \"to_external\": " + r.toExternal);
            w.print("    }");
            w.println(i + 1 == list.size() ? "" : ",");
        }
        w.println("  ]");
    }

    private void writeConstants(PrintWriter w, List<ConstRec> list) {
        Collections.sort(list, new Comparator<ConstRec>() {
            public int compare(ConstRec a, ConstRec b) {
                int c = compareText(a.programPath, b.programPath);
                return c != 0 ? c : compareText(a.address, b.address);
            }
        });
        w.println("  \"constants\": [");
        for (int i = 0; i < list.size(); i++) {
            ConstRec r = list.get(i);
            w.println("    {");
            w.println("      \"program_path\": " + js(r.programPath) + ",");
            w.println("      \"address\": " + js(r.address) + ",");
            w.println("      \"name\": " + js(r.name) + ",");
            w.println("      \"data_type\": " + js(r.dataType) + ",");
            w.println("      \"xref_count\": " + r.xrefCount);
            w.print("    }");
            w.println(i + 1 == list.size() ? "" : ",");
        }
        w.println("  ]");
    }

    private void writeDuplicates(PrintWriter w, Map<String, List<DupEntry>> groups) {
        List<Map.Entry<String, List<DupEntry>>> filtered = new ArrayList<>();
        for (Map.Entry<String, List<DupEntry>> e : groups.entrySet()) {
            if (distinctProgramCount(e.getValue()) > 1) {
                filtered.add(e);
            }
        }
        Collections.sort(filtered, new Comparator<Map.Entry<String, List<DupEntry>>>() {
            public int compare(Map.Entry<String, List<DupEntry>> a,
                               Map.Entry<String, List<DupEntry>> b) {
                int c = b.getValue().size() - a.getValue().size();
                return c != 0 ? c : compareText(a.getKey(), b.getKey());
            }
        });

        w.println("  \"duplicates\": [");
        int gi = 0;
        for (Map.Entry<String, List<DupEntry>> e : filtered) {
            if (gi > 0) w.println(",");
            w.println("    {");
            w.println("      \"sha256\": " + js(e.getKey()) + ",");
            List<DupEntry> entries = e.getValue();
            Collections.sort(entries, new Comparator<DupEntry>() {
                public int compare(DupEntry a, DupEntry b) {
                    int c = compareText(a.programPath, b.programPath);
                    return c != 0 ? c : compareText(a.address, b.address);
                }
            });
            w.println("      \"program_count\": " + distinctProgramCount(entries) + ",");
            w.println("      \"entries\": [");
            for (int i = 0; i < entries.size(); i++) {
                DupEntry d = entries.get(i);
                w.println("        {");
                w.println("          \"program_path\": " + js(d.programPath) + ",");
                w.println("          \"address\": " + js(d.address) + ",");
                w.println("          \"name\": " + js(d.name) + ",");
                w.println("          \"body_min\": " + js(d.bodyMin) + ",");
                w.println("          \"body_max\": " + js(d.bodyMax) + ",");
                w.println("          \"name_source\": " + js(d.nameSource) + ",");
                w.println("          \"signature\": " + js(d.signature) + ",");
                w.println("          \"size\": " + d.size);
                w.print("        }");
                w.println(i + 1 == entries.size() ? "" : ",");
            }
            w.println("      ]");
            w.print("    }");
            gi++;
        }
        if (!filtered.isEmpty()) w.println();
        w.println("  ]");
    }

    /* ---- utilities ---- */

    private int countMultiProgramGroups(Map<String, List<DupEntry>> groups) {
        int c = 0;
        for (List<DupEntry> list : groups.values()) {
            if (distinctProgramCount(list) > 1) c++;
        }
        return c;
    }

    private static int compareText(String a, String b) {
        return s(a).compareTo(s(b));
    }

    private static int distinctProgramCount(List<DupEntry> list) {
        Set<String> programs = new HashSet<>();
        for (DupEntry entry : list) {
            programs.add(s(entry.programPath));
        }
        return programs.size();
    }

    private String sha256Hex(byte[] data) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] digest = md.digest(data);
            StringBuilder sb = new StringBuilder();
            for (byte b : digest) sb.append(String.format("%02x", b));
            return sb.toString();
        } catch (NoSuchAlgorithmException ex) {
            throw new RuntimeException(ex);
        }
    }

    private static String s(String v) { return v == null ? "" : v; }

    private String js(String v) {
        if (v == null) return "null";
        StringBuilder b = new StringBuilder();
        b.append('"');
        for (int i = 0; i < v.length(); i++) {
            char ch = v.charAt(i);
            switch (ch) {
                case '"':  b.append("\\\""); break;
                case '\\': b.append("\\\\"); break;
                case '\b': b.append("\\b"); break;
                case '\f': b.append("\\f"); break;
                case '\n': b.append("\\n"); break;
                case '\r': b.append("\\r"); break;
                case '\t': b.append("\\t"); break;
                default:
                    if (ch < 0x20) b.append(String.format("\\u%04x", (int)ch));
                    else b.append(ch);
            }
        }
        b.append('"');
        return b.toString();
    }
}
