import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import java.util.LinkedHashMap;
import java.util.Map;
public class InspectCvarRegistration extends GhidraScript {
 @Override public void run() throws Exception {long[] a={0x140101640L,0x140100840L,0x140101770L,0x1401010c0L,0x1401022d0L,0x1401022f0L};Map<String,Function> fs=new LinkedHashMap<>();for(long x:a){Function f=getFunctionContaining(toAddr(x));if(f!=null)fs.put(f.getEntryPoint().toString(),f);println("TARGET "+toAddr(x)+" "+(f==null?"none":f.getEntryPoint()));for(Reference r:getReferencesTo(toAddr(x))){Function c=getFunctionContaining(r.getFromAddress());println(" REF "+r.getFromAddress()+" "+(c==null?"none":c.getEntryPoint()+" "+c.getName()));if(c!=null)fs.put(c.getEntryPoint().toString(),c);}}DecompInterface d=new DecompInterface();d.openProgram(currentProgram);for(Function f:fs.values()){DecompileResults r=d.decompileFunction(f,30,monitor);println("\nFUNCTION "+f.getEntryPoint()+" "+f.getName()+" SIZE="+f.getBody().getNumAddresses());if(r.getDecompiledFunction()!=null)println(r.getDecompiledFunction().getC());else println(r.getErrorMessage());}d.dispose();}
}
