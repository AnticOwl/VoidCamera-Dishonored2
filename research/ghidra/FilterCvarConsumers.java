import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import java.util.LinkedHashMap;
import java.util.Map;

public class FilterCvarConsumers extends GhidraScript {
 @Override public void run() throws Exception {
  Map<String,Function> fs=new LinkedHashMap<>();
  for(Reference r:getReferencesTo(toAddr(0x14228BAE8L))){Function f=getFunctionContaining(r.getFromAddress());if(f!=null)fs.put(f.getEntryPoint().toString(),f);}
  println("MANAGER_GLOBAL_REFERENCING_FUNCTIONS="+fs.size());
  DecompInterface d=new DecompInterface();d.openProgram(currentProgram);int candidates=0;
  for(Function f:fs.values()){
   if(monitor.isCancelled())break;
   if(f.getBody().getNumAddresses()>5000)continue;
   DecompileResults r=d.decompileFunction(f,20,monitor);if(!r.decompileCompleted()||r.getDecompiledFunction()==null)continue;
   String c=r.getDecompiledFunction().getC();
   boolean method=c.contains("+ 0x90")||c.contains("+ 0x98")||c.contains("+ 0x88");
   if(method){candidates++;println("\nCANDIDATE "+f.getEntryPoint()+" "+f.getName()+" SIZE="+f.getBody().getNumAddresses()+"\n"+c);}
  }
  println("FILTERED_CANDIDATES="+candidates);d.dispose();
 }
}
