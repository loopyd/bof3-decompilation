// Export all project function symbols to the raw JSON consumed by
// bin/inventory-import-ghidra-symbols.

import ghidra.app.script.GhidraScript;
import ghidra.framework.model.DomainFile;
import ghidra.framework.model.DomainFolder;
import ghidra.framework.model.ProjectData;
import ghidra.program.model.address.AddressSetView;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.listing.Program;
import ghidra.program.model.symbol.SourceType;

import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

public class ExportSymbolsJson extends GhidraScript {
    private static final class Row {
        String programPath;
        String address;
        String name;
        String signature;
        String bodyMin;
        String bodyMax;
        String namespace;
        String nameSource;
        boolean thunk;
        String comment;
        String repeatableComment;
    }

    @Override
    protected void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 1) {
            throw new IllegalArgumentException("usage: ExportSymbolsJson.java <output-json> [project-path]");
        }

        List<Row> rows = new ArrayList<>();
        ProjectData projectData = state.getProject().getProjectData();
        String selectedPath = args.length > 1 ? args[1] : "/";
        collectRows(projectData.getRootFolder(), rows, selectedPath);
        rows.sort(Comparator
            .comparing((Row row) -> row.programPath)
            .thenComparing(row -> row.address)
            .thenComparing(row -> row.name));

        Path outputPath = Path.of(args[0]);
        Path parent = outputPath.getParent();
        if (parent != null) {
            Files.createDirectories(parent);
        }
        try (PrintWriter writer = new PrintWriter(
                Files.newBufferedWriter(outputPath, StandardCharsets.UTF_8))) {
            writeJson(writer, state.getProject().getName(), rows);
        }
    }

    private void collectRows(DomainFolder folder, List<Row> rows, String selectedPath)
            throws Exception {
        monitor.checkCancelled();
        for (DomainFile file : folder.getFiles()) {
            monitor.checkCancelled();
            if (Program.class.isAssignableFrom(file.getDomainObjectClass())
                    && isSelectedProgram(file.getPathname(), selectedPath)) {
                collectProgramRows(file, rows);
            }
        }
        for (DomainFolder child : folder.getFolders()) {
            collectRows(child, rows, selectedPath);
        }
    }

    private boolean isSelectedProgram(String programPath, String selectedPath) {
        if (selectedPath == null || selectedPath.isBlank() || selectedPath.equals("/")) {
            return true;
        }
        String normalized = selectedPath.startsWith("/") ? selectedPath : "/" + selectedPath;
        return programPath.equals(normalized) || programPath.startsWith(normalized + "/");
    }

    private void collectProgramRows(DomainFile file, List<Row> rows) throws Exception {
        Program program = null;
        try {
            program = (Program) file.getDomainObject(this, false, false, monitor);
            FunctionManager functionManager = program.getFunctionManager();
            FunctionIterator functions = functionManager.getFunctions(true);
            while (functions.hasNext()) {
                monitor.checkCancelled();
                Function function = functions.next();
                Row row = new Row();
                row.programPath = file.getPathname();
                row.address = function.getEntryPoint().toString();
                row.name = function.getName();
                row.signature = function.getSignature().getPrototypeString();
                AddressSetView body = function.getBody();
                row.bodyMin = body == null ? null : body.getMinAddress().toString();
                row.bodyMax = body == null ? null : body.getMaxAddress().toString();
                row.namespace = function.getParentNamespace().getName(true);
                SourceType source = function.getSymbol().getSource();
                row.nameSource = source == null ? null : source.name();
                row.thunk = function.isThunk();
                row.comment = function.getComment();
                row.repeatableComment = function.getRepeatableComment();
                rows.add(row);
            }
        }
        finally {
            if (program != null) {
                program.release(this);
            }
        }
    }

    private void writeJson(PrintWriter writer, String projectName, List<Row> rows) {
        writer.println("{");
        writer.println("  \"schema\": \"rebof3-simple.ghidra-symbol-export/v1\",");
        writer.println("  \"project_name\": " + jsonString(projectName) + ",");
        writer.println("  \"rows\": [");
        for (int index = 0; index < rows.size(); index++) {
            Row row = rows.get(index);
            writer.println("    {");
            writer.println("      \"kind\": \"function\",");
            writer.println("      \"program_path\": " + jsonString(row.programPath) + ",");
            writer.println("      \"address\": " + jsonString(row.address) + ",");
            writer.println("      \"name\": " + jsonString(row.name) + ",");
            writer.println("      \"type_spec\": " + jsonString(row.signature) + ",");
            writer.println("      \"body_min\": " + jsonString(row.bodyMin) + ",");
            writer.println("      \"body_max\": " + jsonString(row.bodyMax) + ",");
            writer.println("      \"namespace\": " + jsonString(row.namespace) + ",");
            writer.println("      \"name_source\": " + jsonString(row.nameSource) + ",");
            writer.println("      \"is_thunk\": " + row.thunk + ",");
            writer.println("      \"comment\": " + jsonString(row.comment) + ",");
            writer.println("      \"repeatable_comment\": " + jsonString(row.repeatableComment));
            writer.print("    }");
            writer.println(index + 1 == rows.size() ? "" : ",");
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
                    }
                    else {
                        builder.append(ch);
                    }
                    break;
            }
        }
        builder.append('"');
        return builder.toString();
    }
}
