// Export groups of duplicate functions across programs in the Ghidra project.
// Groups by SHA256 of function body bytes for local duplicate analysis.
//
// Usage (headless):
//   analyzeHeadless <project-dir> <project-name> -process / -recursive \
//     -scriptPath <this-dir> \
//     -postScript ExportDuplicateGroups.java <output-json> [/project-path]
//
// Args:
//   output-json   - path where duplicate_groups.json is written
//   project-path  - optional Ghidra project path filter (default: "/")
//
// Output schema: harness.ghidra-duplicate-groups/v1

import ghidra.app.script.GhidraScript;
import ghidra.framework.model.DomainFile;
import ghidra.framework.model.DomainFolder;
import ghidra.framework.model.ProjectData;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.listing.Program;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.mem.MemoryAccessException;
import ghidra.program.model.symbol.SourceType;

import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ExportDuplicateGroups extends GhidraScript {

    private static final class FuncInfo {
        String programPath;
        String address;
        String name;
        String bodyMin;
        String bodyMax;
        String nameSource;
        String signature;
        int size;
    }

    @Override
    protected void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 1) {
            throw new IllegalArgumentException(
                "usage: ExportDuplicateGroups.java <output-json> [project-path]");
        }

        Map<String, List<FuncInfo>> groups = new HashMap<>();
        ProjectData projectData = state.getProject().getProjectData();
        String selectedPath = args.length > 1 ? args[1] : "/";
        collectGroups(projectData.getRootFolder(), groups, selectedPath);

        // Write JSON
        Path outputPath = Path.of(args[0]);
        Path parent = outputPath.getParent();
        if (parent != null) {
            Files.createDirectories(parent);
        }
        try (PrintWriter writer = new PrintWriter(
                Files.newBufferedWriter(outputPath, StandardCharsets.UTF_8))) {
            writeJson(writer, groups);
        }
        printf("Duplicate groups exported: %d groups from %d functions\n",
               countMultiProgramGroups(groups), countTotalFuncs(groups));
    }

    private void collectGroups(DomainFolder folder, Map<String, List<FuncInfo>> groups,
                               String selectedPath) throws Exception {
        monitor.checkCancelled();
        for (DomainFile file : folder.getFiles()) {
            monitor.checkCancelled();
            if (Program.class.isAssignableFrom(file.getDomainObjectClass())
                    && isSelectedProgram(file.getPathname(), selectedPath)) {
                collectProgramFuncs(file, groups);
            }
        }
        for (DomainFolder child : folder.getFolders()) {
            collectGroups(child, groups, selectedPath);
        }
    }

    private boolean isSelectedProgram(String programPath, String selectedPath) {
        if (selectedPath == null || selectedPath.isBlank() || selectedPath.equals("/")) {
            return true;
        }
        String normalized = selectedPath.startsWith("/") ? selectedPath
                : "/" + selectedPath;
        return programPath.equals(normalized)
                || programPath.startsWith(normalized + "/");
    }

    private void collectProgramFuncs(DomainFile file, Map<String, List<FuncInfo>> groups)
            throws Exception {
        Program program = null;
        try {
            program = (Program) file.getDomainObject(this, false, false, monitor);
            String programPath = file.getPathname();
            FunctionManager functionManager = program.getFunctionManager();
            FunctionIterator functions = functionManager.getFunctions(true);
            Memory memory = program.getMemory();

            while (functions.hasNext()) {
                monitor.checkCancelled();
                Function function = functions.next();

                // Skip IMPORTED / thunk functions (GTE, BIOS, PsyQ stubs)
                SourceType source = function.getSymbol().getSource();
                if (source == SourceType.IMPORTED || function.isThunk()) {
                    continue;
                }

                // Skip functions outside PSX RAM range
                long entryOffset = function.getEntryPoint().getOffset();
                if (entryOffset < 0x80000000L) {
                    continue;
                }

                // Read function body bytes and compute SHA256
                long bodyStart = function.getBody().getMinAddress().getOffset();
                long bodyEnd = function.getBody().getMaxAddress().getOffset();
                int size = (int) (bodyEnd - bodyStart + 1);
                if (size <= 0 || size > 65536) {
                    continue;
                }

                byte[] bytes = new byte[size];
                try {
                    memory.getBytes(function.getBody().getMinAddress(), bytes);
                } catch (MemoryAccessException e) {
                    continue;
                }

                String sha256 = sha256Hex(bytes);
                FuncInfo info = new FuncInfo();
                info.programPath = programPath;
                info.address = function.getEntryPoint().toString();
                info.name = function.getName();
                info.bodyMin = function.getBody().getMinAddress().toString();
                info.bodyMax = function.getBody().getMaxAddress().toString();
                info.nameSource = source.name();
                info.signature = function.getSignature().getPrototypeString();
                info.size = size;
                groups.computeIfAbsent(sha256, k -> new ArrayList<>()).add(info);
            }
        } finally {
            if (program != null) {
                program.release(this);
            }
        }
    }

    private String sha256Hex(byte[] data) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] digest = md.digest(data);
            StringBuilder sb = new StringBuilder();
            for (byte b : digest) {
                sb.append(String.format("%02x", b));
            }
            return sb.toString();
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException(e);
        }
    }

    private int countMultiProgramGroups(Map<String, List<FuncInfo>> groups) {
        int count = 0;
        for (List<FuncInfo> list : groups.values()) {
            if (distinctPrograms(list) > 1) {
                count++;
            }
        }
        return count;
    }

    private int countTotalFuncs(Map<String, List<FuncInfo>> groups) {
        int count = 0;
        for (List<FuncInfo> list : groups.values()) {
            count += list.size();
        }
        return count;
    }

    private int distinctPrograms(List<FuncInfo> list) {
        return (int) list.stream().map(f -> f.programPath).distinct().count();
    }

    private void writeJson(PrintWriter writer, Map<String, List<FuncInfo>> groups) {
        // Filter: only output groups spanning >1 program
        List<Map.Entry<String, List<FuncInfo>>> filtered = new ArrayList<>();
        for (Map.Entry<String, List<FuncInfo>> entry : groups.entrySet()) {
            if (distinctPrograms(entry.getValue()) > 1) {
                filtered.add(entry);
            }
        }
        filtered.sort(Comparator
                .comparingInt((Map.Entry<String, List<FuncInfo>> e) -> -e.getValue().size())
                .thenComparing(e -> e.getKey()));

        writer.println("{");
        writer.println("  \"schema\": \"harness.ghidra-duplicate-groups/v1\",");
        writer.print("  \"groups\": [");

        int groupIdx = 0;
        for (Map.Entry<String, List<FuncInfo>> entry : filtered) {
            if (groupIdx > 0) writer.print(",");
            writer.println();
            writer.println("    {");
            writer.println("      \"sha256\": " + jsonString(entry.getKey()) + ",");
            writer.println("      \"occurrence_count\": " + entry.getValue().size() + ",");
            writer.print("      \"entries\": [");

            List<FuncInfo> list = entry.getValue();
            list.sort(Comparator.comparing(f -> f.programPath));
            for (int i = 0; i < list.size(); i++) {
                if (i > 0) writer.print(",");
                writer.println();
                FuncInfo f = list.get(i);
                writer.println("        {");
                writer.println("          \"program_path\": " + jsonString(f.programPath) + ",");
                writer.println("          \"address\": " + jsonString(f.address) + ",");
                writer.println("          \"name\": " + jsonString(f.name) + ",");
                writer.println("          \"body_min\": " + jsonString(f.bodyMin) + ",");
                writer.println("          \"body_max\": " + jsonString(f.bodyMax) + ",");
                writer.println("          \"name_source\": " + jsonString(f.nameSource) + ",");
                writer.println("          \"signature\": " + jsonString(f.signature) + ",");
                writer.println("          \"size\": " + f.size);
                writer.print("        }");
            }
            writer.println();
            writer.print("      ]");
            writer.println();
            writer.print("    }");
            groupIdx++;
        }
        if (!filtered.isEmpty()) {
            writer.println();
        }
        writer.println("  ]");
        writer.println("}");
    }

    private String jsonString(String value) {
        if (value == null) {
            return "null";
        }
        StringBuilder builder = new StringBuilder();
        builder.append('"');
        for (int index = 0; index < value.length(); index++) {
            char ch = value.charAt(index);
            switch (ch) {
                case '"':
                    builder.append("\\\"");
                    break;
                case '\\':
                    builder.append("\\\\");
                    break;
                case '\b':
                    builder.append("\\b");
                    break;
                case '\f':
                    builder.append("\\f");
                    break;
                case '\n':
                    builder.append("\\n");
                    break;
                case '\r':
                    builder.append("\\r");
                    break;
                case '\t':
                    builder.append("\\t");
                    break;
                default:
                    if (ch < 0x20) {
                        builder.append(String.format("\\u%04x", (int) ch));
                    } else {
                        builder.append(ch);
                    }
                    break;
            }
        }
        builder.append('"');
        return builder.toString();
    }
}
