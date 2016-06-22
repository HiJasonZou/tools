/*=========================================================================
Copyright (c) Constantine Butakoff
All rights reserved.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.
=========================================================================*/
//Remesh-like hole filling
//based on Eurographics Symposium on Geometry Processing(2003), Filling Holes in Meshes, Peter Liepa
//and Filling Gaps in the Boundary of a Polyhedron, Gill Barequet, Micha Sharir

#ifndef __SurfaceHoleFiller_h
#define __SurfaceHoleFiller_h

#include "HoleFillerDefines.h"



class SurfaceHoleFiller
{
public:
    void SetInput(vtkPolyData* mesh) { m_inputMesh = mesh; };

    void SmoothingOn() {m_performCoverSMoothing=true;};
    void SmoothingOff() {m_performCoverSMoothing=false;};
    bool GetSmoothing(){return m_performCoverSMoothing;};
    bool SetSmoothing(bool v){ m_performCoverSMoothing = v;};
    
    void Update();

    
    
    vtkPolyData* GetOutput() {return m_outputMesh; };
    
    SurfaceHoleFiller():m_performCoverSMoothing(true){};
protected:
    
private:
    bool m_performCoverSMoothing;    
    
    vtkSmartPointer<vtkPolyData> m_inputMesh;
    vtkSmartPointer<vtkPolyData> m_outputMesh;
    
    //returns the unordered boundary of all the holes
    void FindHoles(vtkPolyData *mesh, HoleBoundaryType& unordered_edges) const;
    void SplitHoles(HoleBoundaryType& unordered_edges, ArrayOfBoundariesType& hole_boundaries) const;

    //the last vertex is equal to the first vertex to simulate closed loop.
    //so the number of returned vertices is actually the number of edges +1
    void EdgesToVertexArray(const HoleBoundaryType& ordered_boundary, VertexIDArrayType& vertices) const;


    void InitialCoverTriangulation(vtkPolyData* mesh, HoleBoundaryType& ordered_boundary, HoleCoverType& cover) const;

    //for explanation of O, see the paper. This is recursive function Trace
    void PopulateCover(HoleCoverType& cover, vtkIdType i, vtkIdType k, const TriangularIDMatrixType& O, const VertexIDArrayType& vertices) const;

    double TriangleWeightFunctionArea(const VectorType& u, const VectorType& v, const VectorType& w) const;


    //calculates cosine of the dihedral angle. Vertices must have correct orientation
    //v1, v2, v3 - triangle 1
    //u1, u2, u3 - triangle 2
    double CalculateDihedralAngleCos(const VectorType& v1, const VectorType& v2, const VectorType& v3,
            const VectorType& u1, const VectorType& u2, const VectorType& u3) const;



    vtkIdType FindThirdVertexId(vtkPolyData* mesh, vtkIdType p1, vtkIdType p2) const;


    //does m1+m2, as described in the paper
    //(a,b)+(c,d) = (max(a,c),b+d)
    AreaAngleMeasureType SumAreaTriangleMeasures(const AreaAngleMeasureType& m1,
            const AreaAngleMeasureType& m2) const;

    //checks if m1 < m2, lexicographically
    //(a,b)<(c,d) iff (a<c or (a==c and b<d))
    bool AreaTriangleMeasureLess(const AreaAngleMeasureType& m1,
            const AreaAngleMeasureType& m2) const;

    //modifies the mesh
    void RefineCover(vtkPolyData* mesh, const HoleBoundaryType& ordered_boundary, const HoleCoverType& cover) const;
    void SplitRelaxTriangles(vtkPolyData* mesh, VertexIDArrayType& boundaryVertexIDs, HoleCoverType& localCover, vtkPoints* coverVertices) const;

    
    //returns also Vc - centroid, and Svc - centroid's weight
    bool IsTriangleSplitRequired(vtkPoints* coverVertices, const std::vector<double>& sigmas, const vtkIdType idVi, 
            const vtkIdType idVj, const vtkIdType idVk, VectorType& Vc, double& Svc) const;


    //save the cover for debugging mostly. the localCover ids should point to the elements of the 
    //coverVertices array
    void SaveIsolatedCover(const HoleCoverType& localCover, vtkPoints * coverVertices, const char* filename) const;

    void GetVertexNeighbors(vtkPolyData *mesh, vtkIdType vertexId, std::set<vtkIdType>& connectedVertices) const;


    //return false if there is no pair
    bool FindConnectedVertices(vtkPoints* vertices, const HoleCoverType& localCover, const EdgeType& edge, EdgeType& intersectingEdge) const;

    //check if ptcheck is inside a circle defined by the 3 points
    bool IsPointInCircle(const VectorType& pt0, const VectorType& pt1, const VectorType& pt2, const VectorType& ptcheck) const;


    HoleCoverType::iterator FindTriangleByPointIds(HoleCoverType& localCover, vtkIdType id0, vtkIdType id1, vtkIdType id2) const;


    //try to replace edge "edge" with "candidateEdge". Both must have ids into localCover.
    //conn - upper! triangular connectivity matrix, will be updated
    //coverVertices - vertex coordinates of the cover
    //localCover - cover with ids into coverVertices, will be updated
    //returns true if at least one swap was performed
    bool RelaxEdgeIfPossible(const EdgeType& edge, const EdgeType& candidateEdge, vtkPoints* coverVertices, 
                                HoleCoverType& localCover, SparseIDMatrixType& conn) const;

    //returns true if at least one swap was performed
    bool RelaxAllCoverEdges(HoleCoverType& localCover, vtkPoints * coverVertices, SparseIDMatrixType& conn) const;    
    
    bool CheckForDuplicateTriangles(const HoleCoverType& localCover) const;
};


#endif