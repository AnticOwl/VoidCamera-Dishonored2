import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
public class DecompileEntityLod extends GhidraScript {
 @Override public void run() throws Exception {
  DecompInterface d=new DecompInterface();d.openProgram(currentProgram);
  long[] addrs={0x14064cff0L,0x14064bfa0L,0x14031b2e3L};
  for(long v:addrs){Function f=getFunctionContaining(toAddr(v));println("FUNCTION_AT="+toAddr(v)+" ENTRY="+(f==null?"none":f.getEntryPoint()+" "+f.getName()));if(f!=null){DecompileResults r=d.decompileFunction(f,60,monitor);if(r.getDecompiledFunction()!=null)println(r.getDecompiledFunction().getC());else println("FAIL "+r.getErrorMessage());}}
  d.dispose();
 }
}
