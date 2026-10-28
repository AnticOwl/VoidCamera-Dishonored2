import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import java.util.LinkedHashMap;
import java.util.Map;

public class TraceManagerRefs extends GhidraScript {
 @Override public void run() throws Exception {
  long[] globals={0x14228BAE8L,0x1431C0640L,0x141AE6AB8L};
  Map<String,Function> fs=new LinkedHashMap<>();
  for(long g:globals){
   println("GLOBAL "+toAddr(g));
   for(Reference r:getReferencesTo(toAddr(g))){
    Function f=getFunctionContaining(r.getFromAddress());
    println(" REF "+r.getFromAddress()+" type="+r.getReferenceType()+" FUNC="+(f==null?"none":f.getEntryPoint()+" "+f.getName()));
    if(f!=null)fs.put(f.getEntryPoint().toString(),f);
   }
  }
  println("UNIQUE_FUNCTIONS="+fs.size());
  DecompInterface d=new DecompInterface(); d.openProgram(currentProgram);
  for(Function f:fs.values()){
   DecompileResults r=d.decompileFunction(f,30,monitor);
   println("\nFUNCTION "+f.getEntryPoint()+" "+f.getName()+" SIZE="+f.getBody().getNumAddresses());
   if(r.getDecompiledFunction()!=null)println(r.getDecompiledFunction().getC());else println(r.getErrorMessage());
  }
  d.dispose();
 }
}
