import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.symbol.Reference;

public class InspectLodXrefs extends GhidraScript {
  @Override public void run() throws Exception {
    long base = 0x140000000L;
    long[] rvas = {0x32FBF20L, 0x32FBF4CL};
    for (long rva : rvas) {
      Address a = toAddr(base + rva);
      println("TARGET " + a + " RVA=" + Long.toHexString(rva));
      Reference[] refs = getReferencesTo(a);
      println("REFCOUNT=" + refs.length);
      for (Reference ref : refs) {
        Address from = ref.getFromAddress();
        Instruction ins = getInstructionAt(from);
        println("REF " + ref + " INS=" + (ins == null ? "<no instruction>" : ins.toString()));
        if (ins != null) {
          InstructionIterator it = currentProgram.getListing().getInstructions(from, true);
          int n=0;
          while (it.hasNext() && n<7) { Instruction x=it.next(); println("  " + x.getAddress() + "  " + x); n++; }
        }
      }
    }
  }
}
