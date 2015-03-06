// This code is part of the project "Ligra: A Lightweight Graph Processing
// Framework for Shared Memory", presented at Principles and Practice of 
// Parallel Programming, 2013.
// Copyright (c) 2013 Julian Shun and Guy Blelloch
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <algorithm>
#include "parallel.h"
#include "gettime.h"
#include "utils.h"
#include "graph.h"
#include "IO.h"
using namespace std;

//*****START FRAMEWORK*****

//*****VERTEX OBJECT*****
struct vertices {
  intT n, m;
  intT* s;
  bool* d;
  bool isDense;

  // make a singleton vertex in range of n
vertices(intT _n, intT v) 
: n(_n), m(1), d(NULL), isDense(0) {
  s = newA(intT,1);
  s[0] = v;
}
  
  //empty vertex set
vertices(intT _n) : n(_n), m(0), d(NULL), s(NULL), isDense(0) {}

  // make vertices from array of vertex indices
  // n is range, and m is size of array
vertices(intT _n, intT _m, intT* indices) 
: n(_n), m(_m), s(indices), d(NULL), isDense(0) {}

  // make vertices from boolean array, where n is range
vertices(intT _n, bool* bits) 
: n(_n), d(bits), s(NULL), isDense(1)  {
  m = sequence::sum(bits,_n);
}

  // make vertices from boolean array giving number of true values
vertices(intT _n, intT _m, bool* bits) 
: n(_n), m(_m), s(NULL), d(bits), isDense(1)  {}

  // delete the contents
  void del(){
    if (d != NULL) free(d);
    if (s != NULL) free(s);
  }

  intT numRows() { return n; }

  intT numNonzeros() { return m; }

  bool isEmpty() { return m==0; }

  // converts to dense but keeps sparse representation if there
  void toDense() {
    if (d == NULL) {
      d = newA(bool,n);
      {parallel_for(intT i=0;i<n;i++) d[i] = 0;}
      {parallel_for(intT i=0;i<m;i++) d[s[i]] = 1;}
    }
    isDense = true;
  }

  // converts to sparse but keeps dense representation if there
  void toSparse() {
    if (s == NULL) {
      _seq<intT> R = sequence::packIndex(d,n);
      if (m != R.n) {
	cout << "bad stored value of m" << endl; 
	abort();
      }
      s = R.A;
    }
    isDense = false;
  }

  // check for equality
  bool eq (vertices& b) {
    toDense();
    b.toDense();
    bool* c = newA(bool,n);
    {parallel_for (intT i=0; i<b.n; i++) 
	c[i] = (d[i] != b.d[i]);}
    bool equal = (sequence::sum(c,n) == 0);
    free(c);
    return equal;
  }

  void print() {
    if (isDense) {
      cout << "D:";
      for (intT i=0;i<n;i++) if (d[i]) cout << i << " ";
      cout << endl;
    } else {
      cout << "S:";
      for (intT i=0; i<m; i++)	cout << s[i] << " ";
      cout << endl;
    }
  }
};

struct nonNegF{bool operator() (intT a) {return (a>=0);}};

//options to edgeMap for different versions of dense edgeMap (default is DENSE)
enum options { DENSE, DENSE_FORWARD};

//remove duplicate integers in [0,...,n-1]
void remDuplicates(intT* indices, intT* flags, intT m, intT n) {
  //make flags for first time
  if(flags == NULL) {flags = newA(intT,n); 
    {parallel_for(intT i=0;i<n;i++) flags[i]=-1;}
  }
  {parallel_for(intT i=0;i<m;i++){ 
      if(indices[i] != -1 && flags[indices[i]] == -1) 
	CAS(&flags[indices[i]],(intT)-1,i);
    }
  }
  //reset flags
  {parallel_for(intT i=0;i<m;i++){
      if(indices[i] != -1){
	if(flags[indices[i]] == i){ //win
	  flags[indices[i]] = -1; //reset
	}
	else indices[i] = -1; //lost
      }
    }
  }
}

//*****EDGE FUNCTIONS*****

template <class F, class vertex>
  bool* edgeMapDense(graph<vertex> GA, bool* vertices, F f, bool parallel = 0) {
  intT numVertices = GA.n;
  vertex *G = GA.V;
  bool* next = newA(bool,numVertices);
  {   
	  parallel_for (intT i=0; i<numVertices; i++){
    next[i] = 0;
    if (f.cond(i)) { 
      intT d = G[i].getInDegree();
   //  if(!parallel || d < 1000)
     if(d < 1000000)
     {

		for(intT j=0; j<d; j++){
		  intT ngh = G[i].getInNeighbor(j);
		  if (vertices[ngh] && f.update(ngh,i)) next[i] = 1;
		  if(!f.cond(i)) break;
		}
      } 
	   
	   else 
	{
      
	  parallel_for(intT j=0; j<d; j++){
	  intT ngh = G[i].getInNeighbor(j);
	  if (vertices[ngh] && f.updateAtomic(ngh,i)) next[i] = 1;
	  }

	}
/*

	{
	//	std::cout<<"dgree="<<d<<std::endl;
		cilk::reducer_opadd<double> total;
		cilk::reducer_opor<bool> flag;
		parallel_for(intT j=0; j<d; j++)
		{
			intT ngh = G[i].getInNeighbor(j);
			if(vertices[ngh])
			{
				total+=f.update_parallel(ngh);
				flag|=1;
			}
			
		}
		f.set_update(i,total.get_value());
		next[i] = flag.get_value();
	}
*/
	}
    }
    }
  return next;
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1

template <class F, class vertex>
  bool* gather_update_scatter(graph<vertex> GA, bool* vertices,bool* next, F f) {
  intT numVertices = GA.n;
  vertex *G = GA.V;
 
 // parallel_for (intT i=0; i<numVertices; i++)
//	next[i] = 0;
  parallel_for (intT i=0; i<numVertices; i++)
  {
    
  //  if (vertices[i]==1) 
	{ 
		intT d = G[i].getInDegree();
		for(intT j=0; j<d; j++)
		{
		  intT ngh = G[i].getInNeighbor(j);
		  f.apply(ngh,i);

		}
		f.update(i);
/*		if(f.update(i))
		{
			for(intT k=0;k<G[i].getOutDegree();k++)
			{
				next[G[i].getOutNeighbor(k)]=1;
			}	
		}
		 */
	}

  }
  return next;
}













template <class F, class vertex>
bool* edgeMapDenseForward(graph<vertex> GA, bool* vertices, F f) {
  intT numVertices = GA.n;
  vertex *G = GA.V;
  bool* next = newA(bool,numVertices);
  {parallel_for(long i=0;i<numVertices;i++) next[i] = 0;}
  {parallel_for (long i=0; i<numVertices; i++){
    if (vertices[i]) {
      intT d = G[i].getOutDegree();
      if(d < 1000) {
	for(intT j=0; j<d; j++){
	  uintT ngh = G[i].getOutNeighbor(j);
	  if (f.cond(ngh) && f.updateAtomic(i,ngh)) next[ngh] = 1;
	}
      }
      else {
	{parallel_for(intT j=0; j<d; j++){
	  uintT ngh = G[i].getOutNeighbor(j);
	  if (f.cond(ngh) && f.updateAtomic(i,ngh)) next[ngh] = 1;
	  }}
      }
    }
    }}
  return next;
}

template <class F, class vertex>
pair<uintT,intT*> edgeMapSparse(vertex* frontierVertices, intT* indices, 
				uintT* degrees, uintT m, F f, 
				intT remDups=0, intT* flags=NULL) {

  uintT* offsets = degrees;
  uintT outEdgeCount = sequence::plusScan(offsets, degrees, m);

  intT* outEdges = newA(intT,outEdgeCount);
  {parallel_for (intT i = 0; i < m; i++) {
    intT v = indices[i];
    intT o = offsets[i];
    vertex vert = frontierVertices[i]; 
    intT d = vert.getOutDegree();
    if(d < 1000) {
      for (intT j=0; j < d; j++) {
	intT ngh = vert.getOutNeighbor(j);
	if (f.cond(ngh) && f.updateAtomic(v,ngh)) 
	  outEdges[o+j] = ngh;
	else outEdges[o+j] = -1;
      } 
    } else {
      {parallel_for (intT j=0; j < d; j++) {
	intT ngh = vert.getOutNeighbor(j);
	if (f.cond(ngh) && f.updateAtomic(v,ngh)) 
	  outEdges[o+j] = ngh;
	else outEdges[o+j] = -1;
	}} 
    }
    }}

  intT* nextIndices = newA(intT, outEdgeCount);
  if(remDups) remDuplicates(outEdges,flags,outEdgeCount,remDups);

  // Filter out the empty slots (marked with -1)
  uintT nextM = sequence::filter(outEdges,nextIndices,outEdgeCount,nonNegF());
  free(outEdges);
  return pair<uintT,intT*>(nextM, nextIndices);
}

static int edgesTraversed = 0;

// decides on sparse or dense base on number of nonzeros in the active vertices
template <class F, class vertex>
vertices edgeMap(graph<vertex> GA, vertices &V, F f, intT threshold = -1, 
		 char option=DENSE, bool remDups=false) {
  intT numVertices = GA.n;
  uintT numEdges = GA.m;
  if(threshold == -1) threshold = numEdges/20; //default threshold
  vertex *G = GA.V;
  intT m = V.numNonzeros();
  if (numVertices != V.numRows()) {
    cout << "edgeMap: Sizes Don't match" << endl;
    abort();
  }

  // used to generate nonzero indices to get degrees
  uintT* degrees = newA(uintT, m);
  vertex* frontierVertices;
  V.toSparse();
  frontierVertices = newA(vertex,m);
  {parallel_for (long i=0; i < m; i++){
    vertex v = G[V.s[i]];
    degrees[i] = v.getOutDegree();
    frontierVertices[i] = v;
    }}
  uintT outDegrees = sequence::plusReduce(degrees, m);
  edgesTraversed += outDegrees;
  if (outDegrees == 0) return vertices(numVertices);
  if (m + outDegrees > threshold) { 
    V.toDense();
    free(degrees);
    free(frontierVertices);
	
    bool* R = (option == DENSE_FORWARD) ? 
      edgeMapDenseForward(GA,V.d,f) : 
      edgeMapDense(GA, V.d, f, option);
    vertices v1 = vertices(numVertices, R);
    //cout << "size (D) = " << v1.m << endl;
    return  v1;
  } else { 
    pair<uintT,intT*> R = 
      remDups ? 
      edgeMapSparse(frontierVertices, V.s, degrees, V.numNonzeros(), f, 
		    numVertices, GA.flags) :
      edgeMapSparse(frontierVertices, V.s, degrees, V.numNonzeros(), f);
    //cout << "size (S) = " << R.first << endl;
    free(degrees);
    free(frontierVertices);
    return vertices(numVertices, R.first, R.second);
  }
}

//*****VERTEX FUNCTIONS*****

template <class F>
void vertexMap(vertices V, F add) {
  intT n = V.numRows();
  uintT m = V.numNonzeros();
  if(V.isDense) {
    {parallel_for(long i=0;i<n;i++)
	if(V.d[i]) add(i);}
  } else {
    {parallel_for(long i=0;i<m;i++)
	add(V.s[i]);}
  }
}

template <class F>
vertices vertexFilter(vertices V, F filter) {
  intT n = V.numRows();
  uintT m = V.numNonzeros();
  V.toDense();
  bool* d_out = newA(bool,n);
  {parallel_for(long i=0;i<n;i++) d_out[i] = 0;}
  {parallel_for(long i=0;i<n;i++)
      if(V.d[i]) d_out[i] = filter(i);}
  return vertices(n,d_out);
}

//*****WEIGHTED EDGE FUNCTIONS*****

template <class F, class vertex>
  bool* edgeMapDense(wghGraph<vertex> GA, bool* vertices, F f, bool parallel = 0) {
  intT numVertices = GA.n;
  vertex *G = GA.V;
  bool* next = newA(bool,numVertices);
  {parallel_for (long i=0; i<numVertices; i++){
    next[i] = 0;
    if (f.cond(i)) {
      intT d = G[i].getInDegree();
      if(!parallel || d < 1000) {
	for(intT j=0; j<d; j++){
	  intT ngh = G[i].getInNeighbor(j);
	  if(vertices[ngh]){
	    intT edgeLen = G[i].getInWeight(j);
	    if (vertices[ngh] && f.update(ngh,i,edgeLen)) next[i] = 1;
	  }
	  if (!f.cond(i)) break;
	}
      }
      else {
	{parallel_for(intT j=0; j<d; j++){
	  intT ngh = G[i].getInNeighbor(j);
	  if(vertices[ngh]){
	    intT edgeLen = G[i].getInWeight(j);
	    if (vertices[ngh] && f.updateAtomic(ngh,i,edgeLen)) next[i] = 1;
	  }
	  }}
      }
    }
    }
  }
  return next;
}

template <class F, class vertex>
bool* edgeMapDenseForward(wghGraph<vertex> GA, bool* vertices, F f) {
  intT numVertices = GA.n;
  vertex *G = GA.V;
  bool* next = newA(bool,numVertices);
  {parallel_for(long i=0;i<numVertices;i++) next[i] = 0;}
  {parallel_for (long i=0; i<numVertices; i++){
    if (vertices[i]) {
      intT d = G[i].getOutDegree();
      if(d < 1000) {
	for(intT j=0; j<d; j++){
	  intT ngh = G[i].getOutNeighbor(j);
	  intT edgeLen = G[i].getOutWeight(j);
	  if (f.cond(ngh) && f.updateAtomic(i,ngh,edgeLen)) next[ngh] = 1;
	}
      }
      else{
	parallel_for(intT j=0; j<d; j++){
	  intT ngh = G[i].getOutNeighbor(j);
	  intT edgeLen = G[i].getOutWeight(j);
	  if (f.cond(ngh) && f.updateAtomic(i,ngh,edgeLen)) next[ngh] = 1;
	}
      }
    }
  }}
  return next;
}

template <class F, class vertex>
  pair<uintT,intT*> edgeMapSparseW(vertex* frontierVertices, intT* indices, 
				   uintT* degrees, uintT m, F f, 
				   intT remDups=0, intT* flags=NULL) {
  uintT* offsets = degrees;
  uintT outEdgeCount = sequence::plusScan(offsets, degrees, m);

  intT* outEdges = newA(intT,outEdgeCount);

  {parallel_for (long i = 0; i < m; i++) {
    intT v = indices[i];
    uintT o = offsets[i];
    vertex vert = frontierVertices[i];
    intT d = vert.getOutDegree();
    if(d < 1000) {
      for (intT j=0; j < d; j++) {
	intT ngh = vert.getOutNeighbor(j);
	intT edgeLen = vert.getOutWeight(j);
	if (f.cond(ngh) && f.updateAtomic(v,ngh,edgeLen)) 
	  outEdges[o+j] = ngh;
	else outEdges[o+j] = -1;
      }
    }
    else {
      {parallel_for (intT j=0; j < d; j++) {
	intT ngh = vert.getOutNeighbor(j);
	intT edgeLen = vert.getOutWeight(j);
	if (f.cond(ngh) && f.updateAtomic(v,ngh,edgeLen)) 
	  outEdges[o+j] = ngh;
	else outEdges[o+j] = -1;
	
	}}
    }
    }}

  intT* nextIndices = newA(intT, outEdgeCount);
  if(remDups) remDuplicates(outEdges,flags,outEdgeCount,remDups);

  // Filter out the empty slots (marked with -1)
  uintT nextM = sequence::filter(outEdges,nextIndices,outEdgeCount,nonNegF());
  free(outEdges);
  return pair<uintT,intT*>(nextM, nextIndices);
}

// decides on sparse or dense base on number of nonzeros in the active vertices
template <class F, class vertex>
  vertices edgeMap(wghGraph<vertex> GA, vertices V, F f, intT threshold = -1, char option=DENSE, bool remDups=false) {
  intT numVertices = GA.n;
  uintT numEdges = GA.m;
  if(threshold == -1) threshold = numEdges/20; //default threshold
  vertex *G = GA.V;
  intT m = V.numNonzeros();
  if (numVertices != V.numRows()) {
    cout << "edgeMap: Sizes Don't match" << endl;
    abort();
  }

  // used to generate nonzero indices to get degrees
  uintT* degrees = newA(uintT, m);
  vertex* frontierVertices;
  V.toSparse();
  frontierVertices = newA(vertex,m);

  {parallel_for (long i=0; i < m; i++){
    vertex v = G[V.s[i]];
    degrees[i] = v.getOutDegree();
    frontierVertices[i] = v;
    }}
  uintT outDegrees = sequence::plusReduce(degrees, m);    
  if (outDegrees == 0) return vertices(numVertices);
  if (m + outDegrees > threshold) { 
    V.toDense();
    free(degrees);
    free(frontierVertices);
    bool* R = option == DENSE_FORWARD ? 
      edgeMapDenseForward(GA,V.d,f) : 
      edgeMapDense(GA, V.d, f,option);
    vertices v1 = vertices(numVertices, R);
    //cout << "size (D) = " << v1.m << endl;
    return  v1;
  } else { 
    pair<uintT,intT*> R = 
      remDups ? 
      edgeMapSparseW(frontierVertices, V.s, degrees, V.numNonzeros(), f, 
		    numVertices, GA.flags) :
      edgeMapSparseW(frontierVertices, V.s, degrees, V.numNonzeros(), f);
    //cout << "size (S) = " << R.first << endl;
    free(degrees);
    free(frontierVertices);
    return vertices(numVertices, R.first, R.second);
  }
}

inline bool cond_true (intT d) { return 1; }
