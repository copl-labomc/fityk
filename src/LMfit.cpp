// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License ver. 2+

#include "common.h"
#include "LMfit.h"
#include "ui.h"
#include "settings.h"
#include "logic.h"
#include <math.h>
#include <vector>
#include <algorithm>

using namespace std;

LMfit::LMfit(Ftk* F)
    : Fit(F, "levenberg_marquardt")
{
}

LMfit::~LMfit() {}

// WSSR is also called chi2
fp LMfit::init()
{
    alpha.resize (na*na);
    alpha_.resize (na*na);
    beta.resize (na);
    beta_.resize (na);
    lambda = F_->get_settings()->lm_lambda_start;
    a = a_orig;

    F_->vmsg(print_matrix (a, 1, na, "Initial A"));
    //no need to optimise it (and compute chi2 and derivatives together)
    chi2 = compute_wssr(a, dmdm_);
    compute_derivatives(a, dmdm_, alpha, beta);
    return chi2;
}

void LMfit::autoiter()
{
    wssr_before = chi2;
    fp prev_chi2 = chi2;
    const SettingsMgr *sm = F_->settings_mgr();
    F_->vmsg("\t === Levenberg-Marquardt method ===");
    F_->vmsg("Initial values:  lambda=" + S(lambda)
              + "  WSSR=" + sm->format_double(chi2));
    F_->vmsg("Max. number of iterations: " + max_iterations);
    fp stop_rel = F_->get_settings()->lm_stop_rel_change;
    fp max_lambda = F_->get_settings()->lm_max_lambda;
    if (stop_rel > 0) {
        F_->vmsg ("Will stop when relative change of WSSR is "
                  "twice in row below " + S (stop_rel * 100.) + "%");
    }
    int small_change_counter = 0;
    for (int iter = 0; !common_termination_criteria(iter); iter++) {
        bool better_fit = do_iteration();
        if (better_fit) {
            fp d = prev_chi2 - chi2;
            F_->vmsg("#" + S(iter_nr) + ":  WSSR=" + sm->format_double(chi2)
                     + "  lambda=" + S(lambda) + "  d(WSSR)=" +  S(-d)
                     + "  (" + S (d / prev_chi2 * 100) + "%)");
            // another termination criterium: negligible change of chi2
            if (d / prev_chi2 < stop_rel || chi2 == 0) {
                small_change_counter++;
                if (small_change_counter >= 2 || chi2 == 0) {
                    F_->msg("... converged.");
                    break;
                }
            }
            else
                small_change_counter = 0;
            prev_chi2 = chi2;
        }
        else { // no better fit
            F_->vmsg("#" + S(iter_nr) + ": (WSSR=" + sm->format_double(chi2_)
                     + ")  lambda=" + S(lambda));
            if (lambda > max_lambda) { // another termination criterium
                F_->msg("In L-M method: lambda=" + S(lambda) + " > "
                        + S(max_lambda) + ", stopped.");
                break;
            }
        }
        iteration_plot(a, chi2);
    }
    post_fit (a, chi2);
}

bool LMfit::do_iteration()
    //pre: init() callled
{
    if (na < 1)
        throw ExecuteError("No parameters to fit.");
    iter_nr++;
    alpha_ = alpha;
    for (int j = 0; j < na; j++)
        alpha_[na * j + j] *= (1.0 + lambda);
    beta_ = beta;
    if (F_->get_verbosity() > 1) { // level: debug
        F_->msg (print_matrix (beta_, 1, na, "beta"));
        F_->msg (print_matrix (alpha_, na, na, "alpha'"));
    }

    // Matrix solution (Ax=b)  alpha_ * da == beta_
    Jordan (alpha_, beta_, na);

    // da is in beta_
    if (F_->get_verbosity() >= 1) { // level: verbose
        vector<fp> rel (na);
        for (int q = 0; q < na; q++)
            rel[q] = beta_[q] / a[q] * 100;
        F_->vmsg(print_matrix (rel, 1, na, "delta(A)/A[%]"));
    }

    for (int i = 0; i < na; i++)
        beta_[i] = a[i] + beta_[i];   // and now there is new a[] in beta_[]

    if (F_->get_verbosity() >= 1) {
        const SettingsMgr *sm = F_->settings_mgr();
        string s = "Trying A={ ";
        v_foreach (fp, j, beta_)
            s += sm->format_double(*j) + (j+1 == beta_.end() ? " }" : ", ");
        F_->vmsg(s);
    }

    //  compute chi2_
    chi2_ = compute_wssr(beta_, dmdm_);
    //printf("// %g\n", chi2_ - chi2);

    if (chi2_ < chi2) { // better fitting
        chi2 = chi2_;
        a = beta_;
        compute_derivatives(a, dmdm_, alpha, beta);
        lambda /= F_->get_settings()->lm_lambda_down_factor;
        return true;
    }
    else {// worse fitting
        lambda *= F_->get_settings()->lm_lambda_up_factor;
        return false;
    }
}

