import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;

public class InspectLodConsumer extends GhidraScript {
  @Override public void run() throws Exception {
    Address a = toAddr(0x1403522d1L);
    Function f = getFunctionContaining(a);
    println("CONSUMER " + a + " FUNCTION=" + (f == null ? "<unknown>" : f.getEntryPoint()+" "+f.getName()+" size="+f.getBody().getNumAddresses()));
    if (f != null) {
      DecompInterface d = new DecompInterface(); d.openProgram(currentProgram);
      DecompileResults r = d.decompileFunction(f, 90, monitor);
      println("DECOMPILE_OK="+r.decompileCompleted());
      if (r.getDecompiledFunction()!=null) println(r.getDecompiledFunction().getC());
      else println(r.getErrorMessage());
      d.dispose();
    }
    InstructionIterator it = currentProgram.getListing().getInstructions(a, true);
    for (int i=0; i<90 && it.hasNext(); i++) { Instruction x=it.next(); println(x.getAddress()+"  "+x); }
  }
}
