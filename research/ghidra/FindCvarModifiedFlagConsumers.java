import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import java.util.LinkedHashMap;
import java.util.Map;
public class FindCvarModifiedFlagConsumers extends GhidraScript {
 @Override public void run() throws Exception {Map<String,Function> fs=new LinkedHashMap<>();for(Reference r:getReferencesTo(toAddr(0x1431C04B0L))){Function f=getFunctionContaining(r.getFromAddress());if(f!=null)fs.put(f.getEntryPoint().toString(),f);}println("CVAR_REGISTRY_USERS="+fs.size());DecompInterface d=new DecompInterface();d.openProgram(currentProgram);for(Function f:fs.values()){if(f.getBody().getNumAddresses()>5000)continue;DecompileResults r=d.decompileFunction(f,20,monitor);if(r.getDecompiledFunction()==null)continue;String c=r.getDecompiledFunction().getC();if(c.contains("0x40000")||c.contains("0x80000")||c.contains("0x50000")||c.contains("0xd0000")){println("\nFUNCTION "+f.getEntryPoint()+" "+f.getName()+" SIZE="+f.getBody().getNumAddresses()+"\n"+c);}}d.dispose();}
}
