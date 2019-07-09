// ---------------------------------------------------------------------
//
// Copyright (C) 2019 by the deal.II authors
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE.md at
// the top level directory of deal.II.
//
// ---------------------------------------------------------------------

#ifndef dealii_hp_refinement_h
#define dealii_hp_refinement_h


#include <deal.II/base/config.h>

DEAL_II_NAMESPACE_OPEN

// forward declarations
template <typename Number>
class Vector;

namespace hp
{
  template <int dim, int spacedim>
  class DoFHandler;
}

namespace hp
{
  /**
   * We supply adaptive methods to align computational ressources with the
   * complexity of the numerical solution. Error estimates are an appropriate
   * means of determining where adjustments need to be made.
   *
   * However with hp adaptivity, we have two ways to realize these adjustments:
   * For irregular solutions, h adaptive methods which dynamically assign cell
   * sizes tend to reduce the approximation error, while for smooth solutions p
   * adaptive methods are better suited in which function spaces will be
   * selected dynamically. This namespace collects tools to decide which type
   * of adaptive methods to apply.
   *
   * <h3>Usage</h3>
   *
   * To successfully apply hp adaptive methods, we recommend the following
   * workflow:
   * <ol>
   * <li> A suitable error estimate is the basis for any kind of adaptive method.
   * Similar to pure grid refinement, we will determine error estimates in the
   * usual way (i.e. KellyErrorEstimator) and mark cells for refinement or
   * coarsening (i.e. GridRefinement).
   *
   * Calling Triangulation::execute_coarsening_and_refinement() at this stage
   * will perform pure grid refinement as expected.
   *
   * <li> Once all refinement and coarsening flags have been distributed on the
   * mesh, we may determine if those qualify for p adaptive methods.
   * Corresponding functions will set @p future_fe_indices on top of the
   * refinement and coarsening flags if they fulfil a certain criterion.
   *
   * In case of refinement, the superordinate element of the underlying
   * hp::FECollection will be assigned as the future finite element.
   * Correspondingly, the subordinate element will be selected for coarsening.
   *
   * Triangulation::execute_coarsening_and_refinement() will now supply both h
   * and p adaptive methods independently.
   *
   * <li> Right now, there may be cells scheduled for both h and p adaptation.
   * If we do not want to impose both methods at once, we need to decide which
   * one to pick for each cell individually and unambiguously. Since grid
   * refinement will be imposed by default and we only determine qualification
   * for p adaptivity on top, we will always decide in favour of p adaptive
   * methods.
   *
   * Calling Triangulation::execute_coarsening_and_refinement() will now perform
   * either h or p adaptive methods uniquely on each cell.
   *
   * <li> Up to this point, each cell knows its destiny in terms of adaptivity
   * We can now move on to prepare all data structures to be transferred across
   * mesh changes. Previously set refinement and coarsening flags as well as
   * @p future_fe_indices will be used to update the data accordingly.
   * </ol>
   *
   * As an example, a realisation of pure p adaptive methods would look like the
   * following:
   * @code
   * // step 1: flag cells for refinement or coarsening
   * Vector<float> estimated_error_per_cell (triangulation.n_active_cells());
   * KellyErrorEstimator<dim>::estimate (hp_dof_handler,
   *                                     QGauss<dim-1> (quadrature_points),
   *                                     typename FunctionMap<dim>::type(),
   *                                     solution,
   *                                     estimated_error_per_cell);
   * GridRefinement::refine_and_coarsen_fixed_fraction (triangulation,
   *                                                    estimated_error_per_cell,
   *                                                    top_fraction,
   *                                                    bottom_fraction);
   *
   * // step 2: set future finite element indices on flagged cells
   * hp::Refinement::full_p_adaptivity (hp_dof_handler);
   *
   * // step 3: decide whether h or p adaptive methods will be supplied
   * hp::Refinement::force_p_over_h (hp_dof_handler);
   *
   * // step 4: prepare solutions to be transferred
   * ...
   *
   * triangulation.execute_coarsening_and_refinement();
   * @endcode
   *
   * @ingroup hp
   * @author Marc Fehling 2019
   */
  namespace Refinement
  {
    /**
     * @name Setting p adaptivity flags
     * @{
     */

    /**
     * Each cell flagged for h refinement will also be flagged for p refinement.
     * The same applies to coarsening.
     *
     * @note Preceeding calls of Triangulation::prepare_for_coarsening_and_refinement()
     *   may change refine and coarsen flags, which will ultimately change the
     *   results of this function.
     */
    template <int dim, int spacedim>
    void
    full_p_adaptivity(const hp::DoFHandler<dim, spacedim> &dof_handler);

    /**
     * Adapt the finite element on cells that have been specifically flagged for
     * p adaptation via the parameter @p p_flags. Future finite elements will
     * only be assigned if cells have been flagged for refinement and coarsening
     * beforehand.
     *
     * Each entry of the parameter @p p_flags needs to correspond to an active
     * cell.
     *
     * @note Preceeding calls of Triangulation::prepare_for_coarsening_and_refinement()
     *   may change refine and coarsen flags, which will ultimately change the
     *   results of this function.
     */
    template <int dim, int spacedim>
    void
    p_adaptivity_from_flags(const hp::DoFHandler<dim, spacedim> &dof_handler,
                            const std::vector<bool> &            p_flags);

    /**
     * Adapt the finite element on cells whose smoothness indicators meet a
     * certain threshold.
     *
     * The threshold will be chosen for refined and coarsened cells
     * individually. For each class of cells, we determine the maximal and
     * minimal values of the smoothness indicators and determine the threshold
     * by linear interpolation between these limits. Parameters
     * @p p_refine_fraction and @p p_refine_coarsen are used as interpolation
     * factors, where `0` corresponds to the minimal and `1` to the maximal
     * value. By default, mean values are considered as thresholds.
     *
     * We consider a cell for p refinement if it is flagged for refinement and
     * its smoothness indicator is larger than the corresponding threshold. The
     * same applies for p coarsening, but the cell's indicator must be lower
     * than the threshold.
     *
     * Each entry of the parameter @p smoothness_indicators needs to correspond
     * to an active cell. Parameters @p p_refine_fraction and
     * @p p_coarsen_fraction need to be in the interval $[0,1]$.
     *
     * @note Preceeding calls of Triangulation::prepare_for_coarsening_and_refinement()
     *   may change refine and coarsen flags, which will ultimately change the
     *   results of this function.
     */
    template <int dim, typename Number, int spacedim>
    void
    p_adaptivity_from_threshold(
      const hp::DoFHandler<dim, spacedim> &dof_handler,
      const Vector<Number> &               smoothness_indicators,
      const double                         p_refine_fraction  = 0.5,
      const double                         p_coarsen_fraction = 0.5);

    /**
     * Adapt the finite element on cells based on the regularity of the
     * (unknown) analytical solution.
     *
     * With an approximation of the local Sobolev regularity index $k_K$,
     * we may assess to which finite element space our local solution on cell
     * $K$ belongs. Since the regularity index is only an estimate, we won't
     * use it to assign the finite element space directly, but rather consider
     * it as an indicator for adaptation. If a cell is flagged for refinement,
     * we will perform p refinement once it satisfies
     * $k_K > p_{K,\text{super}}$, where $p_{K,\text{super}}$ is
     * the polynomial degree of the finite element superordinate to the
     * currently active element on cell $K$. In case of coarsening, the
     * criterion $k_K < p_{K,\text{sub}}$ has to be met, with
     * $p_{K,\text{sub}}$ the degree of the subordinate element.
     *
     * Each entry of the parameter @p sobolev_indices needs to correspond
     * to an active cell.
     *
     * For more theoretical details see
     * @code{.bib}
     * @article{Houston2005,
     *  author    = {Houston, Paul and S{\"u}li, Endre},
     *  title     = {A note on the design of hp-adaptive finite element
     *               methods for elliptic partial differential equations},
     *  journal   = {{Computer Methods in Applied Mechanics and Engineering}},
     *  volume    = {194},
     *  number    = {2},
     *  pages     = {229--243},
     *  publisher = {Elsevier},
     *  year      = {2005},
     *  doi       = {10.1016/j.cma.2004.04.009}
     * }
     * @endcode
     *
     * @note Preceeding calls of Triangulation::prepare_for_coarsening_and_refinement()
     *   may change refine and coarsen flags, which will ultimately change the
     *   results of this function.
     */
    template <int dim, typename Number, int spacedim>
    void
    p_adaptivity_from_regularity(
      const hp::DoFHandler<dim, spacedim> &dof_handler,
      const Vector<Number> &               sobolev_indices);

    /**
     * Adapt the finite element on cells based on their refinement history
     * or rather the predicted change of their error estimates.
     *
     * If a cell is flagged for adaptation, we will perform p adaptation once
     * the associated error indicators $\eta_{K}^2$ on cell $K$ satisfy
     * $\eta_{K}^2 < \eta_{K,\text{pred}}^2$, where the subscript $\text{pred}$
     * denotes the predicted error. This corresponds to our assumption of
     * smoothness being correct, else h adaptation is supplied.
     *
     * For the very first adapation step, the user needs to decide whether h or
     * p adapatation is supposed to happen. An h-step will be applied with
     * $\eta_{K,\text{pred} = 0$, whereas $\eta_{K,\text{pred} = \infty$ ensures
     * a p-step. The latter may be realised with `std::numeric_limits::max()`.
     *
     * Each entry of the parameter @p error_indicators and @p predicted_errors
     * needs to correspond to an active cell.
     *
     * For more theoretical details see
     * @code{.bib}
     * @article{Melenk2001,
     *  author    = {Melenk, Jens Markus and Wohlmuth, Barbara I.},
     *  title     = {{On residual-based a posteriori error estimation
     *                in hp-FEM}},
     *  journal   = {{Advances in Computational Mathematics}},
     *  volume    = {15},
     *  number    = {1},
     *  pages     = {311--331},
     *  publisher = {Springer US},
     *  year      = {2001},
     *  doi       = {10.1023/A:1014268310921}
     * }
     * @endcode
     *
     * @note Preceeding calls of Triangulation::prepare_for_coarsening_and_refinement()
     *   may change refine and coarsen flags, which will ultimately change the
     *   results of this function.
     */
    template <int dim, typename Number, int spacedim>
    void
    p_adaptivity_from_prediction(
      const hp::DoFHandler<dim, spacedim> &dof_handler,
      const Vector<Number> &               error_indicators,
      const Vector<Number> &               predicted_errors);

    /**
     * @}
     */

    /**
     * @name Decide between h and p adaptivity
     * @{
     */

    /**
     * Choose p adaptivity over h adaptivity in any case.
     *
     * Removes all refine and coarsen flags on cells that have a
     * @p future_fe_index assigned.
     *
     * @note Preceeding calls of Triangulation::prepare_for_coarsening_and_refinement()
     *   may change refine and coarsen flags, which will ultimately change the
     *   results of this function.
     */
    template <int dim, int spacedim>
    void
    force_p_over_h(const hp::DoFHandler<dim, spacedim> &dof_handler);

    /**
     * Choose p adaptivity over h adaptivity whenever it is invoked on all
     * related cells.
     *
     * In case of refinement, information about finite elements will be
     * inherited. Thus we will prefer p refinement over h refinement whenever
     * desired, i.e. clear the refine flag and supply a corresponding
     * @p future_fe_index.
     *
     * However for coarsening, we follow a different approach. Flagging a cell
     * for h coarsening does not ultimately mean that it will be coarsened. Only
     * if a cell and all of its siblings are flagged, they will be merged into
     * their parent cell. If we consider p coarsening on top, we must decide for
     * all siblings together how they will be coarsened. We distinguish between
     * three different cases:
     * <ol>
     * <li> Not all siblings flagged for coarsening: p coarsening<br>
     *   We keep the @p future_fe_indices and clear the coarsen flags
     *   on all siblings.
     * <li> All siblings flagged for coarsening, but not all for
     *   p adaptation: h coarsening<br>
     *   We keep the coarsen flags and clear all @p future_fe_indices
     *   on all siblings.
     * <li> All siblings flagged for coarsening and p adaptation: p coarsening<br>
     *   We keep the @p future_fe_indices and clear the coarsen flags
     *   on all siblings.
     * </ol>
     *
     * @note The function Triangulation::prepare_coarsening_and_refinement()
     *   will clean up all h coarsening flags if they are not shared among
     *   all siblings. In the hp case, we need to bring forward this decision:
     *   If the cell will not be coarsened, but qualifies for p adaptivity,
     *   we have to set all flags accordingly. So this function anticipates
     *   the decision that Triangulation::prepare_coarsening_and_refinement()
     *   would have made later on.
     *
     * @note Preceeding calls of Triangulation::prepare_for_coarsening_and_refinement()
     *   may change refine and coarsen flags, which will ultimately change the
     *   results of this function.
     */
    template <int dim, int spacedim>
    void
    choose_p_over_h(const hp::DoFHandler<dim, spacedim> &dof_handler);

    /**
     * @}
     */
  } // namespace Refinement
} // namespace hp


DEAL_II_NAMESPACE_CLOSE

#endif // dealii_hp_refinement_h