import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import java.util.LinkedHashMap;
import java.util.Map;

public class TracePresetCallbacks extends GhidraScript {
 @Override public void run() throws Exception {
  long[] targets={0x143982EE0L,0x143982EE8L,0x140100840L,0x140102CB0L};Map<String,Function> fs=new LinkedHashMap<>();
  for(long a:targets){println("TARGET "+toAddr(a));Function f=getFunctionContaining(toAddr(a));if(f!=null)fs.put(f.getEntryPoint().toString(),f);for(Reference r:getReferencesTo(toAddr(a))){Function c=getFunctionContaining(r.getFromAddress());println(" REF "+r.getFromAddress()+" "+(c==null?"none":c.getEntryPoint()+" "+c.getName()));if(c!=null)fs.put(c.getEntryPoint().toString(),c);}}
  DecompInterface d=new DecompInterface();d.openProgram(currentProgram);for(Function f:fs.values()){DecompileResults r=d.decompileFunction(f,30,monitor);println("\nFUNCTION "+f.getEntryPoint()+" "+f.getName()+" SIZE="+f.getBody().getNumAddresses());if(r.getDecompiledFunction()!=null)println(r.getDecompiledFunction().getC());else println(r.getErrorMessage());}d.dispose();
 }
}
