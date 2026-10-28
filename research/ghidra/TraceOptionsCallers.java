import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import java.util.LinkedHashMap;
import java.util.Map;
public class TraceOptionsCallers extends GhidraScript {
 @Override public void run() throws Exception {long[] a={0x1405122d0L,0x14050ace0L,0x140512dd0L,0x140502970L};Map<String,Function> fs=new LinkedHashMap<>();for(long x:a){println("TARGET "+toAddr(x));for(Reference r:getReferencesTo(toAddr(x))){Function f=getFunctionContaining(r.getFromAddress());println(" REF "+r.getFromAddress()+" "+(f==null?"none":f.getEntryPoint()+" "+f.getName()));if(f!=null)fs.put(f.getEntryPoint().toString(),f);}}DecompInterface d=new DecompInterface();d.openProgram(currentProgram);for(Function f:fs.values()){if(f.getBody().getNumAddresses()>3000)continue;DecompileResults r=d.decompileFunction(f,30,monitor);println("\nFUNCTION "+f.getEntryPoint()+" "+f.getName()+" SIZE="+f.getBody().getNumAddresses());if(r.getDecompiledFunction()!=null)println(r.getDecompiledFunction().getC());}d.dispose();}
}
