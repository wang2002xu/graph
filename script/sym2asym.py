import sys
import re 




fin=open(sys.argv[1],'r')
v_list=[]
current_vid=-1
max_id=0
for line in fin.readlines():
	if line[0] != '#':
		v_info_line=re.split(r'\s+',line)
#		print v_info_line
		v_id=int(v_info_line[0])
		v_out_edge=int(v_info_line[1])
		if max_id < v_id:
			max_id=v_id
		if max_id < v_out_edge:
			max_id=v_out_edge
		if v_id != current_vid:
			while current_vid!=v_id:
				current_vid=current_vid+1
				v_list.append([])
		v_list[current_vid].append(v_out_edge)
while current_vid<max_id:
	current_vid=current_vid+1
	v_list.append([])
#print v_list
fin.close()
fin=open(sys.argv[1],'r')
for line in fin.readlines():
	if line[0] != '#':
		v_info_line=re.split(r'\s+',line)
#		print v_info_line
		v_id=int(v_info_line[0])
		v_out_edge=int(v_info_line[1])
		v_list[v_out_edge].append(v_id)

#print v_list

for nbr in v_list:
	nbr.sort();
#print v_list
fin.close()

out_str=[]

for i in range(len(v_list)):
	for j in range(len(v_list[i])):
		out_str.append(str(i)+" "+str(v_list[i][j])+"\n")
#print out_str

fout=open(sys.argv[2],'w')
fout.writelines(out_str)
fout.close()		

