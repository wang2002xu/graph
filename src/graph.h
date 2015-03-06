#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "parallel.h"
#include<algorithm>
using namespace std;

// **************************************************************
//    ADJACENCY ARRAY REPRESENTATION
// **************************************************************

struct symmetricVertex {
  intE* neighbors;
  intT degree;
  void del() {free(neighbors); }
symmetricVertex(intE* n, intT d) : neighbors(n), degree(d) {}
  uintE getInNeighbor(intT j) { return neighbors[j]; }
  uintE getOutNeighbor(intT j) { return neighbors[j]; }
  intT getInDegree() { return degree; }
  intT getOutDegree() { return degree; }
  void setInNeighbors(intE* _i) { neighbors = _i; }
  void setOutNeighbors(intE* _i) { neighbors = _i; }
  void setInDegree(intT _d) { degree = _d; }
  void setOutDegree(intT _d) { degree = _d; }
  void flipEdges() {}
};

struct asymmetricVertex {
  intE* inNeighbors;
  intE* outNeighbors;
  intT outDegree;
  intT inDegree;
  void del() {free(inNeighbors); free(outNeighbors);}
asymmetricVertex(intE* iN, intE* oN, intT id, intT od) : inNeighbors(iN), outNeighbors(oN), inDegree(id), outDegree(od) {}
 inline uintE getInNeighbor(intT j) { return inNeighbors[j]; }
 inline uintE getOutNeighbor(intT j) { return outNeighbors[j]; }
 inline intT getInDegree() { return inDegree; }
 inline intT getOutDegree() { return outDegree; }
  void setInNeighbors(intE* _i) { inNeighbors = _i; }
  void setOutNeighbors(intE* _i) { outNeighbors = _i; }
  void setInDegree(intT _d) { inDegree = _d; }
  void setOutDegree(intT _d) { outDegree = _d; }
  void flipEdges() { swap(inNeighbors,outNeighbors); swap(inDegree,outDegree); }
};

struct v_degree
{
	intT ID;
	intT degree;
};

struct dl_item//double link
{
	intT ID;
	dl_item * prev;
	dl_item * next;
};
	
class map
{
	intT n;
	dl_item *item_arr;
	bool * flag;
	dl_item *top;
public:
	map(int _n)
	{
		n=_n;
		item_arr=new dl_item[n];
		flag= new bool[n];
		top=& item_arr[0];
		for(intT i=0;i<n;i++)
	    {
			flag[i]=0;
			item_arr[i].ID=i;
			if(i==0)
				item_arr[i].prev=NULL;
			else
				item_arr[i].prev=&item_arr[i-1];
			if(i==n-1)
				item_arr[i].next=NULL;
			else
				item_arr[i].next=&item_arr[i+1];  
	    }
		
	}
	void delete_item(int k)
	{
		flag[k]=1;
		dl_item *del_item=&item_arr[k];
		if(top==del_item) //delete top
		{
			if(top->next==NULL)
				top=NULL;
			else
			{	
				top=top->next;
				top->prev=NULL;
			}	
		}
		else
		{
		
			del_item->prev->next=del_item->next;
			if(del_item->next!=NULL)
				del_item->next->prev=del_item->prev;
			
		}	
	}
	
	intT retrieval()
	{
		flag[top->ID]=1;
		dl_item * temp=top;
		if(top->next==NULL)
			top=NULL;
		else
		{	
			top=top->next;
			top->prev=NULL;
		}	
		return temp->ID;
	}
	
	bool is_empty()
	{
		if(top==NULL)
			return 1;
		else
			return 0;
	}
	
	bool get_flag(intT i)
	{
		return flag[i];
	}
	
	~map()
	{
		delete []item_arr;
		delete []flag;
		}
	
	
	
	

	  
};


 bool compare(v_degree a, v_degree b)
  {
	  return a.degree>b.degree;
  }



template <class vertex>
struct graph {
  vertex *V;
  intT* mapping; //bfs order --> original order
  long n;
  long m;
  intE* allocatedInplace; //out edge array
  intE* inEdges;          //in edge array   
  intT* flags;
  graph(vertex* VV, long nn, long mm) 
  : V(VV), n(nn), m(mm), allocatedInplace(NULL), flags(NULL) {}
graph(vertex* VV, long nn, long mm, intE* ai, intE* _inEdges = NULL) 
: V(VV), n(nn), m(mm), allocatedInplace(ai), inEdges(_inEdges), flags(NULL) {}
  void del() {
    if (flags != NULL) free(flags);
    if (allocatedInplace == NULL) 
      for (intT i=0; i < n; i++) V[i].del();
    else free(allocatedInplace);
    free(V);
    if(inEdges != NULL) free(inEdges);
  }
  void transpose() {
    if(sizeof(vertex) == sizeof(asymmetricVertex)) {
      parallel_for(intT i=0;i<n;i++) {
	V[i].flipEdges();
      }
    } 
  }
  
  void print_graph()
  {
	  cout<<"total number of vertices is "<<n<<endl;
	  cout<<"total number of edges is "<<m<<endl;
	  for(intT i=0;i<n;i++)
	  {
			if(V[i].getOutDegree()==0 && V[i].getInDegree()==0)
				cout<<"dangling vertex "<<i<<endl;
	  }
	  for(intT i=0;i<n;i++)
	  {	
		cout<<"vertex_"<<i<<", out_degree="<<V[i].getOutDegree()<<": ";
		for(intT j=0;j<V[i].getOutDegree();j++)
		{
			cout<<V[i].getOutNeighbor(j)<<" ";
		}
		cout<<endl;
	  }
 	  
	  for(intT i=0;i<n;i++)
		{	
			cout<<"vertex_"<<i<<", in_degree="<<V[i].getInDegree()<<": ";
			for(intT j=0;j<V[i].getInDegree();j++)
			{
				cout<<V[i].getInNeighbor(j)<<" ";
			}
			cout<<endl;
		}
	  for(intE i=0;i<m;i++)
		  cout<<allocatedInplace[i]<<" ";
	cout<<endl;	  
/*	  for(intE i=0;i<m;i++)
		  cout<<inEdges[i]<<" ";  
	cout<<endl;
	  */
}
  
  bool BFS(intT & _top,intT & _len,map & v_map)
  {
	  intT top=_top;
	  intT len=_len;
	  _top=top+len;
	  _len=0;
	  for(intT i=top;i<top+len;i++)
	  {
		intT v=mapping[i];
	
	//	cout<<endl<<v<<": "<<endl;
		
		intT in_degree=V[v].getInDegree();
		for(intT j=0;j<in_degree;j++)
		{
			intT in_nbr=V[v].getInNeighbor(j);
			if(v_map.get_flag(in_nbr)==0)
			{
				v_map.delete_item(in_nbr);
				mapping[_top+_len]=in_nbr;
				_len++;
			//	cout<<"inqueue:"<<in_nbr<<"  ";
			}
		}
		
		
	  }
	  return 0;
  }
  void BFS_mapping()
  {
	  
	  
	  map v_map(n);
	  mapping=new intT[n];
	  intT  _top=0;
	  intT  _len=1; 
		  
	  intT max_in_degree=0;
	  intT max_ID=-1;
	/*  for(intT i=0;i<n;i++)
	  {
		  if(V[i].getOutDegree()==0&&V[i].getInDegree()>max_in_degree)
		  {
			  max_in_degree=V[i].getInDegree();
			  max_ID=i;
		  }
	  }
	*/
	  if(max_ID==-1)
		  max_ID=0;
		  
	//  cout<<"max_in_degree="<<max_in_degree<<" max_ID="<<max_ID<<endl;
	  mapping[0]=max_ID;
	  v_map.delete_item(max_ID);
	  
	  int iter=0;
	  int new_root=0;
	  while(1)
	  {
		  iter++;
		//  cout<<endl<<"iter_"<<iter<<":----------------------------------------------------------"<<endl;
		  if(_len==0)
		  {
			  new_root++;
			 // cout<<"new root_"<<new_root<<":"<<endl;
			  if(v_map.is_empty())
				  break;
			  else
			  {
				  intT id=v_map.retrieval();
				  _len++;
				  mapping[_top]=id;
			  }
		  }
		  int brk=BFS(_top,_len,v_map);
	  }
	//cout<<"iter="<<iter<<" new_root="<<new_root<<endl; 	  
  }
  
 
  
   void degree_mapping()
  {
	  
	  mapping=new intT[n];
	  struct v_degree * v_degree=new struct v_degree[n];
	  for(int i=0;i<n;i++)
	  {
		  v_degree[i].ID=i;
		  v_degree[i].degree=V[i].getInDegree();
	  }
	  sort(v_degree,v_degree+n,compare);
	  for(int i=0;i<n;i++)
	  {
		  mapping[i]=v_degree[i].ID;
	  }
	  for(int i=0;i<n;i++)
	  {
		 // cout<<"id="<<v_degree[i].ID<<" degree="<<v_degree[i].degree<<endl;
	  }
	  delete[] v_degree;
  }
  
  void partition_mapping( intT block_size)
  {
	  intT p=n/block_size+1; //partition number
	  cout<<"p="<<p<<endl;
	  intT* par_index=new intT[n];//index which partition one vertex belongs to
	  for(intT i=0;i<n;i++)
		par_index[i]=-1;
	  double* par_pref=new double[p];//the preference of partition one vertex prefers to
	  for(intT i=0;i<p;i++)
		  par_pref[i]=0;
	  intT* par_num=new intT[p];//current number of vertices in partition
	  for(intT i=0;i<p;i++)
		  par_num[i]=0;
	
	  for(int v=0;v<n;v++)
	  {
	    for(intT i=0;i<p;i++)
			par_pref[i]=0.00001;
		intT in_degree=V[v].getInDegree();
		for(intT j=0;j<in_degree;j++)
		{
			intT in_nbr=V[v].getInNeighbor(j);
			if(par_index[in_nbr]!=-1)
				par_pref[par_index[in_nbr]]++;	
	    }
		intT max_par=0;
		double max_par_pref=-1;
		for(intT i=0;i<p;i++)
		{
			double cur_par_pref=par_pref[i]*(1-(double)par_num[i]/(double)block_size);
			//cout<<"cur_par_pref["<<i<<"]="<<cur_par_pref<<endl;
			if(cur_par_pref>max_par_pref)
			{
				max_par_pref=cur_par_pref;
				max_par=i;
			}
		}
		par_index[v]=max_par;
		par_num[max_par]++;
		//cout<<"vertex_"<<v<<"--->partition_"<<max_par<<endl;
	  }
	  
	  mapping=new intT[n];
	  intT ** p_par_curr=new intT*[p];//point to current point for partition p
	  intT * p_head=mapping;
	  for(intT i=0;i<p;i++)
	  {
		  p_par_curr[i]=p_head;
		  p_head=p_head+par_num[i];
	  }
	  for(intT i=0;i<n;i++)
	  {
		  intT par=par_index[i];
		  *p_par_curr[par]=i;
		  p_par_curr[par]++;
	  } 
	  
	  delete[] p_par_curr;
	  delete[] par_pref;
	  delete[] par_num;
		
	  
  }
  
  void random_mapping()
  {
	  mapping=new intT[n];
	 
	  for(int i=0;i<n;i++)
		mapping[i]=i;
	random_shuffle(mapping,mapping+n);
  }
  
  
  void ID_remapping()
  {
	  intT * re_mapping=new intT[n]; //old-->new
	  for(intT i=0;i<n;i++)
	  {
		  re_mapping[mapping[i]]=i;
	  }
	/*  
	    cout<<"new-->old:  ";
	    for(intT i=0;i<n;i++)
		  cout<<mapping[i]<<" ";
		cout<<endl; 
		cout<<"old-->new:  ";
		for(intT i=0;i<n;i++)
		  cout<<re_mapping[i]<<" ";
		cout<<endl; 
		*/
		
	  intT p_in=0;
	  intT p_out=0;
	  intE* new_inEdges=new intE[m];
	  intE* new_outEdges=new intE[m];
	  vertex* new_V = newA(vertex,n);
	  for(intT i=0;i<n;i++)
	  {
		  vertex v=V[mapping[i]];
		  for(intT j=0;j<v.getInDegree();j++)
		  {
			  intT in_nbr=v.getInNeighbor(j);
			  in_nbr=re_mapping[in_nbr];
			  new_inEdges[p_in++]=in_nbr;
		  } 
		  for(intT j=0;j<v.getOutDegree();j++)
		  {
			  intT out_nbr=v.getOutNeighbor(j);
			  out_nbr=re_mapping[out_nbr];
			  new_outEdges[p_out++]=out_nbr;
		  }
		  v.setInNeighbors(&new_inEdges[p_in-v.getInDegree()]);
		  v.setOutNeighbors(&new_outEdges[p_out-v.getOutDegree()]);
		  new_V[i]=v;
	  }	  
	  del();
	  delete []re_mapping;
	  delete []mapping;
	  V=new_V;
	  allocatedInplace=new_outEdges; //out edge array
	  inEdges=new_inEdges;          //in edge array 
  }
  
};


struct symmetricWghVertex {
  //weights are stored in the entry after the neighbor ID
  //so size of neighbor list is twice the degree
  intE* neighbors; 
  intT degree;
  void del() {free(neighbors);}
symmetricWghVertex(intE* n, intT d) : neighbors(n), degree(d) {}
  intE getInNeighbor(intT j) { return neighbors[2*j]; }
  intE getOutNeighbor(intT j) { return neighbors[2*j]; }
  intE getInWeight(intT j) { return neighbors[2*j+1]; }
  intE getOutWeight(intT j) { return neighbors[2*j+1]; }
  intT getInDegree() { return degree; }
  intT getOutDegree() { return degree; }
  void setInNeighbors(intE* _i) { neighbors = _i; }
  void setOutNeighbors(intE* _i) { neighbors = _i; }
  void setInDegree(intT _d) { degree = _d; }
  void setOutDegree(intT _d) { degree = _d; }
};

struct asymmetricWghVertex {
  //weights are stored in the entry after the neighbor ID
  //so size of neighbor list is twice the degree
  intE* inNeighbors; 
  intE* outNeighbors;
  intT inDegree;
  intT outDegree;
  void del() {free(inNeighbors); free(outNeighbors);}
asymmetricWghVertex(intE* iN, intE* oN, intT id, intT od) : inNeighbors(iN), outNeighbors(oN), inDegree(id), outDegree(od) {}
  intE getInNeighbor(intT j) { return inNeighbors[2*j]; }
  intE getOutNeighbor(intT j) { return outNeighbors[2*j]; }
  intE getInWeight(intT j) { return inNeighbors[2*j+1]; }
  intE getOutWeight(intT j) { return outNeighbors[2*j+1]; }
  intT getInDegree() { return inDegree; }
  intT getOutDegree() { return outDegree; }
  void setInNeighbors(intE* _i) { inNeighbors = _i; }
  void setOutNeighbors(intE* _i) { outNeighbors = _i; }
  void setInDegree(intT _d) { inDegree = _d; }
  void setOutDegree(intT _d) { outDegree = _d; }
};

template <class vertex>
struct wghGraph {
  vertex *V;
  long n;
  long m;
  intE* allocatedInplace;
  intE* inEdges;
  intT* flags;
wghGraph(vertex* VV, long nn, long mm) 
: V(VV), n(nn), m(mm), allocatedInplace(NULL), flags(NULL) {}
wghGraph(vertex* VV, long nn, long mm, intE* ai, intE* _inEdges=NULL) 
: V(VV), n(nn), m(mm), allocatedInplace(ai), inEdges(_inEdges), flags(NULL) {}
  void del() {
    if(flags != NULL) free(flags);
    if (allocatedInplace == NULL) 
      for (intT i=0; i < n; i++) V[i].del();
    else { free(allocatedInplace); }
    free(V);
    if(inEdges != NULL) free(inEdges);
  }
};
