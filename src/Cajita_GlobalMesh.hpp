/****************************************************************************
 * Copyright (c) 2019-2020 by the Cajita authors                            *
 * All rights reserved.                                                     *
 *                                                                          *
 * This file is part of the Cajita library. Cajita is distributed under a   *
 * BSD 3-clause license. For the licensing terms see the LICENSE file in    *
 * the top-level directory.                                                 *
 *                                                                          *
 * SPDX-License-Identifier: BSD-3-Clause                                    *
 ****************************************************************************/

#ifndef CAJTIA_GLOBALMESH_HPP
#define CAJTIA_GLOBALMESH_HPP

#include <Cajita_Types.hpp>

#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>

namespace Cajita
{
//---------------------------------------------------------------------------//
// Forward declaration of global mesh.
template <class MeshType>
class GlobalMesh;

//---------------------------------------------------------------------------//
// Global mesh partial specialization for uniform mesh.
template <class Scalar>
class GlobalMesh<UniformMesh<Scalar>>
{
  public:
    // Mesh type.
    using mesh_type = UniformMesh<Scalar>;

    // Scalar type.
    using scalar_type = Scalar;

    // Cell size constructor.
    GlobalMesh( const std::array<Scalar, 3> &global_low_corner,
                const std::array<Scalar, 3> &global_high_corner,
                const Scalar cell_size )
        : _global_low_corner( global_low_corner )
        , _global_high_corner( global_high_corner )
        , _cell_size( cell_size )
    {
        // Check that the domain is evenly divisible by the cell size in each
        // dimension within round-off error.
        for ( int d = 0; d < 3; ++d )
        {
            Scalar ext = globalNumCell( d ) * _cell_size;
            if ( std::abs( ext - extent( d ) ) >
                 Scalar( 100.0 ) * std::numeric_limits<Scalar>::epsilon() )
                throw std::logic_error(
                    "Extent not evenly divisible by uniform cell size" );
        }
    }

    // Number of global cells constructor.
    GlobalMesh( const std::array<Scalar, 3> &global_low_corner,
                const std::array<Scalar, 3> &global_high_corner,
                const std::array<int, 3> &global_num_cell )
        : _global_low_corner( global_low_corner )
        , _global_high_corner( global_high_corner )
    {
        // Compute the cell size. This should be the same in all dimensions.
        std::array<Scalar, 3> cell_sizes;
        for ( int d = 0; d < 3; ++d )
            cell_sizes[d] = ( _global_high_corner[d] - _global_low_corner[d] ) /
                            global_num_cell[d];
        if ( ( std::abs( cell_sizes[Dim::I] - cell_sizes[Dim::J] ) >
               Scalar( 100.0 ) * std::numeric_limits<Scalar>::epsilon() ) ||
             ( std::abs( cell_sizes[Dim::I] - cell_sizes[Dim::K] ) >
               Scalar( 100.0 ) * std::numeric_limits<Scalar>::epsilon() ) )
            throw std::logic_error( "Cell sizes not equal" );

        // Set the cell size now that we have checked them.
        _cell_size = cell_sizes[Dim::I];

        // Check that the domain is evenly divisible by the cell size in each
        // dimension within round-off error and that we got the expected
        // number of cells.
        for ( int d = 0; d < 3; ++d )
        {
            Scalar ext = globalNumCell( d ) * _cell_size;
            if ( std::abs( ext - extent( d ) ) >
                 Scalar( 100.0 ) * std::numeric_limits<Scalar>::epsilon() )
                throw std::logic_error(
                    "Extent not evenly divisible by uniform cell size" );
            if ( globalNumCell( d ) != global_num_cell[d] )
                throw std::logic_error( "Global number of cells mismatch" );
        }
    }

    // GLOBAL MESH INTERFACE

    // Get the global low corner of the mesh.
    Scalar lowCorner( const int dim ) const { return _global_low_corner[dim]; }

    // Get the global high corner of the mesh.
    Scalar highCorner( const int dim ) const
    {
        return _global_high_corner[dim];
    }

    // Get the extent of a given dimension.
    Scalar extent( const int dim ) const
    {
        return highCorner( dim ) - lowCorner( dim );
    }

    // Get the global numer of cells in a given dimension.
    int globalNumCell( const int dim ) const
    {
        return std::rint( extent( dim ) / _cell_size );
    }

    // UNIFORM MESH SPECIFIC

    // get the uniform cell size.
    Scalar uniformCellSize() const { return _cell_size; }

  private:
    std::array<Scalar, 3> _global_low_corner;
    std::array<Scalar, 3> _global_high_corner;
    Scalar _cell_size;
};

// Creation function.
template <class Scalar>
std::shared_ptr<GlobalMesh<UniformMesh<Scalar>>>
createUniformGlobalMesh( const std::array<Scalar, 3> &global_low_corner,
                         const std::array<Scalar, 3> &global_high_corner,
                         const Scalar cell_size )
{
    return std::make_shared<GlobalMesh<UniformMesh<Scalar>>>(
        global_low_corner, global_high_corner, cell_size );
}

template <class Scalar>
std::shared_ptr<GlobalMesh<UniformMesh<Scalar>>>
createUniformGlobalMesh( const std::array<Scalar, 3> &global_low_corner,
                         const std::array<Scalar, 3> &global_high_corner,
                         const std::array<int, 3> &global_num_cell )
{
    return std::make_shared<GlobalMesh<UniformMesh<Scalar>>>(
        global_low_corner, global_high_corner, global_num_cell );
}

//---------------------------------------------------------------------------//
// Global mesh partial specialization for non-uniform mesh.
template <class Scalar>
class GlobalMesh<NonUniformMesh<Scalar>>
{
  public:
    // Mesh type.
    using mesh_type = NonUniformMesh<Scalar>;

    // Scalar type.
    using scalar_type = Scalar;

    // Constructor.
    GlobalMesh( const std::vector<Scalar> &i_edges,
                const std::vector<Scalar> &j_edges,
                const std::vector<Scalar> &k_edges )
        : _edges( {i_edges, j_edges, k_edges} )
    {
    }

    // GLOBAL MESH INTERFACE

    // Get the global low corner of the mesh.
    Scalar lowCorner( const int dim ) const { return _edges[dim].front(); }

    // Get the global high corner of the mesh.
    Scalar highCorner( const int dim ) const { return _edges[dim].back(); }

    // Get the extent of a given dimension.
    Scalar extent( const int dim ) const
    {
        return highCorner( dim ) - lowCorner( dim );
    }

    // Get the global numer of cells in a given dimension.
    int globalNumCell( const int dim ) const { return _edges[dim].size() - 1; }

    // NON-UNIFORM MESH SPECIFIC

    // Get the edge array in a given dimension.
    const std::vector<Scalar> &nonUniformEdge( const int dim ) const
    {
        return _edges[dim];
    }

  private:
    std::array<std::vector<Scalar>, 3> _edges;
};

// Creation function.
template <class Scalar>
std::shared_ptr<GlobalMesh<NonUniformMesh<Scalar>>>
createNonUniformGlobalMesh( const std::vector<Scalar> &i_edges,
                            const std::vector<Scalar> &j_edges,
                            const std::vector<Scalar> &k_edges )
{
    return std::make_shared<GlobalMesh<NonUniformMesh<Scalar>>>(
        i_edges, j_edges, k_edges );
}

//---------------------------------------------------------------------------//

} // end namespace Cajita

#endif // end CAJTIA_GLOBALMESH_HPP
