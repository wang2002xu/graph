import os 


name=["128k","256k","512k","1m","2m","4m","8m","16m","32m"]
init=131072

n_vertex=[] #number of vertex
for n in range(len(name)):
	n_vertex.append(init)
	init=init*2
print n_vertex

if not os.path.exists('./power'):
	os.mkdir('./power')
if not os.path.exists('./random'):
	os.mkdir('./random')

for i in range(len(name)):
	print("synthesize power_graph_"+name[i]+":")
	os.system("./graphgen -n:"+str(n_vertex[i])+" -g:p -p:1.9 -o:power_"+name[i])
	os.system("python sym2asym.py power_"+name[i]+" sym_power_"+name[i])
	os.system("python data_transform.py sym_power_"+name[i]+" ./power/power"+name[i])
	os.remove("power_"+name[i])
	os.remove("sym_power_"+name[i])
	print("synthesize random_graph_"+name[i]+":")
	os.system("./graphgen -n:"+str(n_vertex[i])+" -g:k -k:16 -o:random_"+name[i])
	os.system("python sym2asym.py random_"+name[i]+" sym_random_"+name[i])
	os.system("python data_transform.py sym_random_"+name[i]+" ./random/random"+name[i])
	os.remove("random_"+name[i])
	os.remove("sym_random_"+name[i])

	
	
