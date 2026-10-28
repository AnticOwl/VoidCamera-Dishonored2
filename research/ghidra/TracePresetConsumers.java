import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

public class TracePresetConsumers extends GhidraScript {
 @Override public void run() throws Exception {
  Map<String,Function> fs=new LinkedHashMap<>();
  long[] addrs={0x143982E70L,0x143982E88L,0x143982E98L};
  for(long a:addrs){println("TARGET "+toAddr(a));for(Reference r:getReferencesTo(toAddr(a))){Function f=getFunctionContaining(r.getFromAddress());println(" REF "+r.getFromAddress()+" "+(f==null?"none":f.getEntryPoint()+" "+f.getName()));if(f!=null)fs.put(f.getEntryPoint().toString(),f);}}
  DataIterator it=currentProgram.getListing().getDefinedData(true);while(it.hasNext()){Data x=it.next();if(!x.getDataType().getName().toLowerCase(Locale.ROOT).contains("string"))continue;Object v=x.getValue();if(v==null||!v.toString().toLowerCase(Locale.ROOT).contains("r_viewdistpreset"))continue;println("STRING "+x.getAddress()+" "+v);for(Reference r:getReferencesTo(x.getAddress())){Function f=getFunctionContaining(r.getFromAddress());println(" STRREF "+r.getFromAddress()+" "+(f==null?"none":f.getEntryPoint()+" "+f.getName()));if(f!=null)fs.put(f.getEntryPoint().toString(),f);}}
  println("PRESET_REFERENCING_FUNCTIONS="+fs.size());DecompInterface d=new DecompInterface();d.openProgram(currentProgram);
  for(Function f:fs.values()){DecompileResults r=d.decompileFunction(f,30,monitor);println("\nFUNCTION "+f.getEntryPoint()+" "+f.getName()+" SIZE="+f.getBody().getNumAddresses());if(r.getDecompiledFunction()!=null)println(r.getDecompiledFunction().getC());else println(r.getErrorMessage());}
  d.dispose();
 }
}
