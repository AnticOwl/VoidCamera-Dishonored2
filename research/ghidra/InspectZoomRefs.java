import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.symbol.Reference;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

public class InspectZoomRefs extends GhidraScript {
    @Override
    public void run() throws Exception {
        Map<String, Function> funcs = new LinkedHashMap<>();
        DataIterator it = currentProgram.getListing().getDefinedData(true);
        String[] needles = {"spyglass", "zoom", "scope", "lod", "fov", "optic", "binocular", "camera"};
        int hits = 0;
        while (it.hasNext() && !monitor.isCancelled()) {
            Data d = it.next();
            if (!d.getDataType().getName().toLowerCase(Locale.ROOT).contains("string")) continue;
            Object value = d.getValue();
            if (value == null) continue;
            String s = value.toString();
            String lower = s.toLowerCase(Locale.ROOT);
            boolean match = false;
            for (String n : needles) if (lower.contains(n)) { match = true; break; }
            if (!match || s.length() < 3 || s.length() > 300) continue;
            println("STRING " + d.getAddress() + " [" + d.getDataType().getName() + "] " + s.replace("\n", " "));
            hits++;
            for (Reference ref : getReferencesTo(d.getAddress())) {
                Function f = currentProgram.getFunctionManager().getFunctionContaining(ref.getFromAddress());
                println("  XREF " + ref.getFromAddress() + " -> " + (f == null ? "<no function>" : f.getEntryPoint()+" "+f.getName()));
                if (f != null && funcs.size() < 80) funcs.put(f.getEntryPoint().toString(), f);
            }
        }
        println("MATCHING_STRINGS=" + hits + " FUNCTIONS=" + funcs.size());
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        int done = 0;
        for (Function f : funcs.values()) {
            if (monitor.isCancelled() || done++ >= 80) break;
            DecompileResults r = decomp.decompileFunction(f, 30, monitor);
            println("\nFUNCTION " + f.getEntryPoint() + " " + f.getName());
            if (r.decompileCompleted() && r.getDecompiledFunction() != null)
                println(r.getDecompiledFunction().getC());
            else println("DECOMPILER: " + r.getErrorMessage());
        }
        decomp.dispose();
    }
}
