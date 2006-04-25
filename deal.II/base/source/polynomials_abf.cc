//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2004, 2005 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------

#include <base/polynomials_abf.h>
#include <base/quadrature_lib.h>
#include <iostream>
#include <iomanip>
using namespace std;
using namespace Polynomials;


template <int dim>
PolynomialsABF<dim>::PolynomialsABF (const unsigned int k)
		:
		my_degree(k),
		n_pols(compute_n_pols(k))
{
  std::vector<std::vector< Polynomials::Polynomial< double > > > pols(dim);
  pols[0] = Polynomials::LagrangeEquidistant::generate_complete_basis(k+2);
  if (k == 0)
    for (unsigned int d=1;d<dim;++d)
      pols[d] = Polynomials::Legendre::generate_complete_basis(0);
  else
    for (unsigned int d=1;d<dim;++d)
      pols[d] = Polynomials::LagrangeEquidistant::generate_complete_basis(k);
  polynomial_space = new AnisotropicPolynomials<dim>(pols);
}


template <int dim>
PolynomialsABF<dim>::~PolynomialsABF ()
{
  delete polynomial_space;
}


template <int dim>
void
PolynomialsABF<dim>::compute (const Point<dim>            &unit_point,
			      std::vector<Tensor<1,dim> > &values,
			      std::vector<Tensor<2,dim> > &grads,
			      std::vector<Tensor<3,dim> > &grad_grads) const
{
  Assert(values.size()==n_pols || values.size()==0,
	 ExcDimensionMismatch(values.size(), n_pols));
  Assert(grads.size()==n_pols|| grads.size()==0,
	 ExcDimensionMismatch(grads.size(), n_pols));
  Assert(grad_grads.size()==n_pols|| grad_grads.size()==0,
	 ExcDimensionMismatch(grad_grads.size(), n_pols));

  const unsigned int n_sub = polynomial_space->n();
  p_values.resize((values.size() == 0) ? 0 : n_sub);
  p_grads.resize((grads.size() == 0) ? 0 : n_sub);
  p_grad_grads.resize((grad_grads.size() == 0) ? 0 : n_sub);
  
  for (unsigned int d=0;d<dim;++d)
    {
				       // First we copy the point. The
				       // polynomial space for
				       // component d consists of
				       // polynomials of degree k+1 in
				       // x_d and degree k in the
				       // other variables. in order to
				       // simplify this, we use the
				       // same AnisotropicPolynomial
				       // space and simply rotate the
				       // coordinates through all
				       // directions.
      Point<dim> p;
      for (unsigned int c=0;c<dim;++c)
	p(c) = unit_point((c+d)%dim);
      
      polynomial_space->compute (p, p_values, p_grads, p_grad_grads);
      
      for (unsigned int i=0;i<p_values.size();++i)
	  values[i+d*n_sub][d] = p_values[i];
      
      for (unsigned int i=0;i<p_grads.size();++i)
	for (unsigned int d1=0;d1<dim;++d1)
	  grads[i+d*n_sub][d][(d1+d)%dim] = p_grads[i][d1];
      
      for (unsigned int i=0;i<p_grad_grads.size();++i)
	for (unsigned int d1=0;d1<dim;++d1)
	  for (unsigned int d2=0;d2<dim;++d2)
	    grad_grads[i+d*n_sub][d][(d1+d)%dim][(d2+d)%dim]
	      = p_grad_grads[i][d1][d2];
    }
}


template <int dim>
unsigned int
PolynomialsABF<dim>::compute_n_pols(unsigned int k)
{
  if (dim == 1) return k+1;
  if (dim == 2) return 2*(k+1)*(k+3);
  //TODO:Check what are the correct numbers ...
  if (dim == 3) return 3*(k+1)*(k+1)*(k+2);
  
  Assert(false, ExcNotImplemented());
  return 0;
}


template class PolynomialsABF<1>;
template class PolynomialsABF<2>;
template class PolynomialsABF<3>;

