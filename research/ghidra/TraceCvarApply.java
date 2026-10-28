import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.symbol.Reference;
import java.util.LinkedHashMap;
import java.util.Map;

public class TraceCvarApply extends GhidraScript {
 @Override public void run() throws Exception {
  long[] targets={0x140102cb0L,0x140102400L,0x1401028e0L,0x14013ff40L};Map<String,Function> fs=new LinkedHashMap<>();
  for(long a:targets){Function f=getFunctionContaining(toAddr(a));if(f!=null)fs.put(f.getEntryPoint().toString(),f);println("TARGET "+toAddr(a)+" FUNC="+(f==null?"none":f.getEntryPoint()+" "+f.getName()));for(Reference r:getReferencesTo(toAddr(a))){Function c=getFunctionContaining(r.getFromAddress());println(" XREF "+r.getFromAddress()+" "+(c==null?"none":c.getEntryPoint()+" "+c.getName()));if(c!=null)fs.put(c.getEntryPoint().toString(),c);}}
  DecompInterface d=new DecompInterface();d.openProgram(currentProgram);for(Function f:fs.values()){DecompileResults r=d.decompileFunction(f,40,monitor);println("\nFUNCTION "+f.getEntryPoint()+" "+f.getName()+" SIZE="+f.getBody().getNumAddresses());if(r.getDecompiledFunction()!=null)println(r.getDecompiledFunction().getC());else println(r.getErrorMessage());InstructionIterator it=currentProgram.getListing().getInstructions(f.getBody(),true);int n=0;while(it.hasNext()&&n++<320){Instruction i=it.next();if(i.getMnemonicString().equals("CALL")||i.getMnemonicString().equals("JMP"))println(" CALLSITE "+i.getAddress()+" "+i);}}
  d.dispose();
 }
}
