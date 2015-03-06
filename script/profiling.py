import os
import re
vtune_path="/opt/intel/vtune_amplifier_xe_2015.1.0.367959/bin64/amplxe-cl" 



if not os.path.exists('./result'):
	os.mkdir('./result')
if not os.path.exists('./report'):
	os.mkdir('./report')

def vtune_profiling(program,data):
	inst_name=re.split(r'/',program)[-1]+"_"+re.split(r'/',data)[-1]
	os.system(vtune_path+" -collect-with runsa -knob event-config=CPU_CLK_UNHALTED.THREAD:sa=2000003,DTLB_LOAD_MISSES.MISS_CAUSES_A_WALK:sa=100003,DTLB_LOAD_MISSES.STLB_HIT:sa=100003,DTLB_LOAD_MISSES.WALK_DURATION:sa=2000003,INST_RETIRED.ANY:sa=2000003,MEM_LOAD_UOPS_LLC_MISS_RETIRED.LOCAL_DRAM:sa=100007,OFFCORE_REQUESTS_OUTSTANDING.CYCLES_WITH_DATA_RD:sa=2000003,OFFCORE_REQUESTS_OUTSTANDING.DEMAND_DATA_RD:sa=2000003 -knob collectMemBandwidth=true -data-limit=0  -result-dir ./result/"+inst_name+" -- "+program+" -s "+data)
	os.system(vtune_path+r' -report hw-events -format csv -csv-delimiter comma -result-dir ./result/'+inst_name+" -report-output ./report/"+inst_name+r'.csv'+r' -filter function=memtest\<symmetricVertex\> ')	
	

program_list=["./random_memtest_iter32","./BFS_memtest_iter32","./degree_memtest_iter32","./256Kpartition_memtest_iter32"]
data_list=[]
graph_data_dir=["/data/wangxu/synthesized_data/power","/data/wangxu/synthesized_data/random"]
for g_dir in  graph_data_dir:
	for g_data in os.listdir(g_dir):
		data_list.append(g_dir+"/"+g_data)

for prog in program_list:
	for data in data_list:
		vtune_profiling(prog,data)

f_final=open('./final_result.csv','w')

for report in os.listdir("./report"):
	f_report=open("./report/"+report)
	head=f_report.readline()
	pd=f_report.readline()
	info=report.split('.')[0].split('_')
	if f_final.tell()==0:	
		f_final.write("order,app,iter,graph_type,vertex_number,edge_number,"+head)
	f_final.write(info[0]+","+info[1]+","+info[2]+","+info[3]+","+info[4]+","+info[5]+","+pd)
	f_report.close()
				

f_final.close()
#prog="./pagerank"
#data="/data/wangxu/synthesized_data/power/power_1m"
#vtune_profiling(prog,data)




