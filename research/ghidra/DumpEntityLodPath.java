import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import java.util.ArrayList;
public class DumpEntityLodPath extends GhidraScript {
 @Override public void run() throws Exception {
  Address a=toAddr(0x14064d0d2L); Function f=getFunctionContaining(a);
  println("ENTRY="+(f==null?"none":f.getEntryPoint()+" "+f.getName()+" BODY_ADDRESSES="+f.getBody().getNumAddresses()));
  ArrayList<Instruction> prev=new ArrayList<>();Instruction p=getInstructionBefore(a);while(p!=null&&prev.size()<35){prev.add(p);p=getInstructionBefore(p.getAddress());}
  for(int i=prev.size()-1;i>=0;i--)println(prev.get(i).getAddress()+" "+prev.get(i));
  InstructionIterator it=currentProgram.getListing().getInstructions(a,true);int n=0;while(it.hasNext()&&n<110){Instruction x=it.next();println(x.getAddress()+" "+x);n++;}
 }
}
