import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
public class DumpLodConsumer extends GhidraScript {
 @Override public void run() throws Exception {
  Address a=toAddr(0x1403522d1L); Function f=getFunctionContaining(a);
  println("ENTRY="+(f==null?"none":f.getEntryPoint()+" "+f.getName()));
  InstructionIterator it=currentProgram.getListing().getInstructions(a,true); int n=0;
  while(it.hasNext()&&n<100){Instruction x=it.next(); println(x.getAddress()+" "+x); n++;}
  println("COUNT="+n);
 }
}
