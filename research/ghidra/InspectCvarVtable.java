import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
public class InspectCvarVtable extends GhidraScript {
 @Override public void run() throws Exception {long[] a={0x140100b00L,0x1400268a0L,0x140101490L,0x1401022f0L,0x1401013e0L,0x1401012e0L,0x1401014a0L,0x1401026d0L,0x140102580L,0x140102660L,0x1401025f0L,0x140101610L,0x140101570L,0x1401015e0L,0x1401015a0L,0x140100f20L,0x1401010c0L,0x140102940L,0x1400f5f80L,0x140598e00L,0x140102320L,0x140102fc0L,0x140101f70L,0x140102700L,0x140102210L,0x140100ee0L,0x1401020a0L,0x140101770L};DecompInterface d=new DecompInterface();d.openProgram(currentProgram);for(int i=0;i<a.length;i++){Function f=getFunctionContaining(toAddr(a[i]));if(f==null){println("SLOT "+i+" NOFUNC "+toAddr(a[i]));continue;}DecompileResults r=d.decompileFunction(f,20,monitor);String c=r.getDecompiledFunction()==null?"":r.getDecompiledFunction().getC();if(c.contains("+ 0x70")||c.contains("+ 0x78")||i==0||i==2||i==3||i==11||i==12||i==13||i==14||i==18||i==19){println("\nSLOT "+i+" RVA="+Long.toHexString(a[i]-0x140000000L)+" ENTRY="+f.getEntryPoint()+" SIZE="+f.getBody().getNumAddresses()+"\n"+c);}}d.dispose();}
}
