/*=========================================================================
Copyright (c) Constantine Butakoff
All rights reserved.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

/*! \file
    \brief Given mesh and flattening, calculates distortion
*/
#include "vtkPolyDataReader.h"
#include "vtkPolyData.h"
#include "vtkPolyDataWriter.h"
#include "vtkType.h"
#include "vtkSmartPointer.h"
#include "vtkCell.h"
#include "VTKCommonTools.h"
#include "Eigen/Dense"



double TriangleGradientMatrix(Eigen::Vector2d& q0, Eigen::Vector2d& q1, Eigen::Vector2d& q2, Eigen::MatrixXd& T,  bool normalize=true);
double TriangleGradientMatrix(vtkPolyData* mesh, vtkIdType pt1id, vtkIdType pt2id, vtkIdType pt3id, Eigen::MatrixXd& T,  bool normalize=true);
void FlattenTriangle(Eigen::Vector3d& p0, Eigen::Vector3d& p1, Eigen::Vector3d& p2, Eigen::Vector2d& q0, Eigen::Vector2d& q1, Eigen::Vector2d& q2);



int main( int argc, char *argv[] )
{	

    if( argc < 5 )
    {
            std::cout << "Usage: " << std::endl;
            std::cout << argv[0] << " <original_mesh> <flattened_mesh> <output_flattened_mesh>" << std::endl;
            std::cout << "flattened_mesh must have only x,y coordinates. Z coordinate is ignored" << std::endl;
            return EXIT_FAILURE;
    }

    const char* mesh3d_filename = argv[1];
    const char* mesh2d_filename = argv[2];
    const char* output_filename = argv[3];

    std::cout << "reading original mesh " << mesh3d_filename << std::endl;
    vtkSmartPointer<vtkPolyData> sourcePd = vtkSmartPointer<vtkPolyData>::Take(
            CommonTools::LoadShapeFromFile(mesh3d_filename) );

    std::cout << "reading flattened mesh " << mesh2d_filename << std::endl;
    vtkSmartPointer<vtkPolyData> targetPd = vtkSmartPointer<vtkPolyData>::Take(
            CommonTools::LoadShapeFromFile(mesh2d_filename) );


    //CommonTools::SaveShapeToFile(targetPd,output_filename);

    return 0;
}





/*! \brief Calculate the gradient matrix for a 2D triangle (q0,q1,q2)
 * T = y2-y3  y3-y1  y1-y2 == TX
 *     x2-X3  x3-x1  x1-x2 == TY
 * See "Least Squares Conformal Maps for Automatic Texture Atlas Generation" by Levy
 * 
 *  @param normalize - (default true) if false, the matrix is not normalized by the area (if you plan to do it later)
 * 
 *  @return W 2x3 matrix, the return value is twice the triangle area. 
 */
double TriangleGradientMatrix(Eigen::Vector2d& q0, Eigen::Vector2d& q1, Eigen::Vector2d& q2, Eigen::MatrixXd& T,  bool normalize)
{
    T.resize(2,3);
    T(0,0) = q1(1) - q2(1);
    T(0,1) = q2(1) - q0(1);
    T(0,2) = q0(1) - q1(1);

    T(1,0) = q1(0) - q2(0);
    T(1,1) = q2(0) - q0(0);
    T(1,2) = q0(0) - q1(0);

    
    //double the area of the triangle
    double area2 =  (q0(0)*q1(1) - q0(1)*q1(0)) + 
                    (q1(0)*q2(1) - q1(1)*q2(0)) + 
                    (q2(0)*q0(1) - q2(1)*q0(0));    
    
    if(normalize)
        T/=area2;
    
    return area2;
}


/*! \brief Calculate the gradient matrix for a 3D triangle (q0,q1,q2)
 * Projects the triangle to 2D and calls the corresponding function
 * 
 *  @param normalize - (default true) if false, the matrix is not normalized by the area (if you plan to do it later)
 * 
 *  @return W 2x3 matrix, the return value is twice the triangle area. 
 */
double TriangleGradientMatrix(vtkPolyData* mesh, vtkIdType pt1id, vtkIdType pt2id, vtkIdType pt3id, Eigen::MatrixXd& T,  bool normalize)
{
    Eigen::Vector3d pt1_R3;
    Eigen::Vector3d pt2_R3;
    Eigen::Vector3d pt3_R3;

    mesh->GetPoint(pt1id, pt1_R3.data());
    mesh->GetPoint(pt2id, pt2_R3.data());
    mesh->GetPoint(pt3id, pt3_R3.data());

    //project the points to the local coordinate system
    //define the axes
    Eigen::Vector2d pt1_R2, pt2_R2, pt3_R2;
    FlattenTriangle(pt1_R3, pt2_R3, pt3_R3, pt1_R2, pt2_R2, pt3_R2);

    return TriangleGradientMatrix(pt1_R2, pt2_R2, pt3_R2, T, normalize);   
}


/*! \brief Flatten the 3D triangle (p0,p1,p2)
 * Axes: v0 = p1-p0, v0 x (p2-p1) x v0
 * Origin: p0
 * 
 *  @return q0, q1, q2
 */
void FlattenTriangle(Eigen::Vector3d& p0, Eigen::Vector3d& p1, Eigen::Vector3d& p2, Eigen::Vector2d& q0, Eigen::Vector2d& q1, Eigen::Vector2d& q2)
{
    //project the points to the local coordinate system
    //define the axes
    Eigen::Vector3d v0 = p2 - p1;
    double pt12_dist = v0.norm();
    v0.normalize();
    Eigen::Vector3d  v1 = (v0.cross(p2-p0)).cross (v0);
    v1.normalize();

    //in local coordinates v0, v1
    //pt1 will have coordinates (0,0)
    //pt2 will have coordinates ( ||pt2-pt1||, 0  ), because it lies on v0
    //pt3 will have coordinates ( v0.(pt3-pt1), v1.(pt3-pt1) )
    q0.fill(0);    
    q1.fill(0);
    
    q1(0) = pt12_dist;

    q2(0) = v0.dot(p2 - p0);
    q2(1) = v1.dot(p2 - p0);    
}
