import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Instruction;
import java.util.ArrayList;
public class DumpLodPreceding extends GhidraScript {
 @Override public void run() throws Exception {
  Address a=toAddr(0x1403522d1L); ArrayList<Instruction> xs=new ArrayList<>(); Instruction x=getInstructionBefore(a);
  while(x!=null&&xs.size()<80){xs.add(x); x=getInstructionBefore(x.getAddress());}
  for(int i=xs.size()-1;i>=0;i--) println(xs.get(i).getAddress()+" "+xs.get(i));
 }
}
