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
import ghidra.program.model.listing.Parameter;
import ghidra.program.model.listing.Program;
import ghidra.program.model.listing.Variable;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;
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
        String kind;
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
        String fromAddress;
        String toAddress;
        String referenceType;
        int operandIndex;
        boolean externalReference;
        List<VariableRow> parameters = new ArrayList<>();
        List<VariableRow> locals = new ArrayList<>();
    }

    private static final class VariableRow {
        String name;
        String dataType;
        String storage;
        int ordinal;
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
            .comparing((Row row) -> sortKey(row.programPath))
            .thenComparing(row -> sortKey(row.kind))
            .thenComparing(row -> sortKey(row.address))
            .thenComparing(row -> sortKey(row.fromAddress))
            .thenComparing(row -> sortKey(row.toAddress))
            .thenComparing(row -> sortKey(row.name)));

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
                row.kind = "function";
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
                for (Parameter parameter : function.getParameters()) {
                    row.parameters.add(variableRow(parameter));
                }
                for (Variable local : function.getLocalVariables()) {
                    row.locals.add(variableRow(local));
                }
                rows.add(row);
            }
            SymbolIterator symbols = program.getSymbolTable().getAllSymbols(true);
            while (symbols.hasNext()) {
                monitor.checkCancelled();
                Symbol symbol = symbols.next();
                if (symbol.getSymbolType().toString().equals("Function")) {
                    continue;
                }
                Row row = new Row();
                row.kind = "symbol";
                row.programPath = file.getPathname();
                row.address = symbol.getAddress().toString();
                row.name = symbol.getName();
                row.namespace = symbol.getParentNamespace().getName(true);
                SourceType source = symbol.getSource();
                row.nameSource = source == null ? null : source.name();
                rows.add(row);
            }
            ReferenceIterator references = program.getReferenceManager()
                    .getReferenceIterator(program.getMemory().getMinAddress());
            while (references.hasNext()) {
                monitor.checkCancelled();
                Reference reference = references.next();
                Row row = new Row();
                row.kind = "xref";
                row.programPath = file.getPathname();
                row.fromAddress = reference.getFromAddress().toString();
                row.toAddress = reference.getToAddress().toString();
                row.referenceType = reference.getReferenceType().getName();
                row.operandIndex = reference.getOperandIndex();
                row.externalReference = reference.isExternalReference();
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
            writer.println("      \"kind\": " + jsonString(row.kind) + ",");
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
            writer.println("      \"repeatable_comment\": " + jsonString(row.repeatableComment) + ",");
            writer.println("      \"from_address\": " + jsonString(row.fromAddress) + ",");
            writer.println("      \"to_address\": " + jsonString(row.toAddress) + ",");
            writer.println("      \"reference_type\": " + jsonString(row.referenceType) + ",");
            writer.println("      \"operand_index\": " + row.operandIndex + ",");
            writer.println("      \"external_reference\": " + row.externalReference + ",");
            writer.println("      \"parameters\": " + variablesJson(row.parameters) + ",");
            writer.println("      \"locals\": " + variablesJson(row.locals));
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

    private VariableRow variableRow(Variable variable) {
        VariableRow row = new VariableRow();
        row.name = variable.getName();
        row.dataType = variable.getDataType() == null ? null : variable.getDataType().getDisplayName();
        row.storage = variable.getVariableStorage() == null ? null : variable.getVariableStorage().toString();
        row.ordinal = variable instanceof Parameter ? ((Parameter) variable).getOrdinal() : -1;
        return row;
    }

    private String variablesJson(List<VariableRow> variables) {
        StringBuilder builder = new StringBuilder();
        builder.append('[');
        for (int index = 0; index < variables.size(); index++) {
            VariableRow variable = variables.get(index);
            if (index > 0) {
                builder.append(',');
            }
            builder.append('{');
            builder.append("\"name\":").append(jsonString(variable.name)).append(',');
            builder.append("\"data_type\":").append(jsonString(variable.dataType)).append(',');
            builder.append("\"storage\":").append(jsonString(variable.storage)).append(',');
            builder.append("\"ordinal\":").append(variable.ordinal);
            builder.append('}');
        }
        builder.append(']');
        return builder.toString();
    }

    private String sortKey(String value) {
        return value == null ? "" : value;
    }
}
