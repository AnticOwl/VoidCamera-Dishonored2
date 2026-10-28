import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.symbol.Reference;
public class InspectDistanceRefs extends GhidraScript {
 @Override public void run() throws Exception {
  long base=0x140000000L;
  long[] rvas={0x3988BDC,0x3988C5C,0x3988CDC,0x3988D5C,0x3988DDC,0x3988E5C,0x32FBF4CL};
  String[] names={"lod_forcevisibledistance","lod_dist_level0","lod_dist_level1","lod_dist_level2","lod_dist_level3","lod_dist_far","r_lodZoomMultiplicator"};
  for(int i=0;i<rvas.length;i++){
   Address a=toAddr(base+rvas[i]); Reference[] refs=getReferencesTo(a); println(names[i]+" TARGET="+a+" REFCOUNT="+refs.length);
   for(Reference ref:refs){Instruction ins=getInstructionAt(ref.getFromAddress());println("  "+ref.getFromAddress()+" "+(ins==null?"<no instruction>":ins));}
  }
 }
}
