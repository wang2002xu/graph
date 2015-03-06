import os

data_list=[]
graph_data_dir=["/data/wangxu/synthesized_data/power","/data/wangxu/synthesized_data/random"]
for g_dir in  graph_data_dir:
	for g_data in os.listdir(g_dir):
		fpath=g_dir+"/"+g_data
		fd=open(fpath,'r')
		if fd.readline()!="AdjacencyGraph\n":
			print "error: format is not compatible!"
		else:
			n=fd.readline().strip('\n') 
			m=fd.readline().strip('\n') 
			os.system("mv "+fpath+" "+g_dir+"/"+g_data+"_"+n+"_"+m)

