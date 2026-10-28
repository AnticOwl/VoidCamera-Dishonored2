import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
public class DecompileLodChooser extends GhidraScript {
 @Override public void run() throws Exception {Function f=getFunctionContaining(toAddr(0x14064bfa0L));println("ENTRY="+(f==null?"none":f.getEntryPoint()+" "+f.getName()+" ADDRESSES="+f.getBody().getNumAddresses()));if(f!=null){DecompInterface d=new DecompInterface();d.openProgram(currentProgram);DecompileResults r=d.decompileFunction(f,60,monitor);if(r.getDecompiledFunction()!=null)println(r.getDecompiledFunction().getC());else println(r.getErrorMessage());d.dispose();}}
}
