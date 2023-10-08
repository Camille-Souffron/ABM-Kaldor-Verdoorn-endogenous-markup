//#define EIGENLIB			// uncomment to use Eigen linear algebra library

#include "fun_head_fast.h"

// do not add Equations in this area




MODELBEGIN

// insert your equations here, ONLY between the MODELBEGIN and MODELEND words





// MICRO PART: FIRM LEVEL



EQUATION("z_firm")
/*
Firms market share in the global economy
$z_{i,j,t} = z_{i,j,t - 1} \left(1 + \phi \left( \frac{E_{i,j,t}}{\bar{E}_t} - 1 \right)\right)$
*/

v[0] = VL("z_firm", 1);
v[1] = V("phi_global");
v[2] = V("E_firm");
v[3] = V("E_global");

v[4] = v[0] * (1 + v[1] * (v[2] / v[3] - 1));

RESULT(v[4])



EQUATION("E_firm")
/*
Level of competitiveness of firms (fitness)
$E_{i,j,t} = \frac{1}{p_{i,j,t}}$
*/

v[0] = V("p_firm");

if (v[0] == 0 || isnan(v[0])) {
    v[1] = 0;  // or some other default value
} else {
    v[1] = 1 / v[0];
}

RESULT(v[1])




EQUATION("p_firm")
/*
Price setting of firms product
$p_{i,j,t} = (1 + \mu_j) \frac{w_{j,t - 1}}{A_{i,j,t - 1}}$
*/

v[0] = V("mu_economy");
v[1] = VL("w_economy", 1);
v[2] = VL("A_firm", 1);

v[3] = (1 + v[0]) * v[1] / v[2];

RESULT(v[3])



EQUATION("A_firm")
/*
Labour productivity of firms
$A_{i,j,t} = \frac{I_{i,j,t}}{K_{i,j,t}} a_{i,j,t - 1} + \frac{K_{i,j,t - 1}}{K_{i,j,t}} A_{i,j,t - 1}$
Using the formulation from @lorentz2018 [p. 187]
*/

v[0] = V("I_firm");
v[1] = V("K_firm");
v[2] = VL("a_firm", 1);
v[3] = VL("A_firm", 1);
v[4] = VL("I_firm", 1);
v[6] = VL("K_firm", 1);

v[5] = v[0] / v[1] * v[2] + v[6] / v[1] * v[3];

RESULT(v[5])



EQUATION("K_firm")
/*
Capital = sum of investments over time for a single firm
$K_{i,j,t} = \sum_{\tau = 1}^t I_{i,j,\tau}$
*/

v[0] = VL("K_firm", 1);
v[1] = V("I_firm");

v[2] = v[0] + v[1];

RESULT(v[2])



EQUATION("Pi_firm")
/*
Profit level of firms
$\Pi_{i,j,t} = p_{i,j,t} Y_{i,j,t} - w_{j,t - 1} L_{i,j,t} = \mu_j \frac{w_{j,t - 1}}{A_{i,j,t - 1}} Y_{i,j,t}$
*/

v[0] = V("mu_economy");
v[1] = VL("w_economy", 1);
v[2] = VL("A_firm", 1);
v[3] = V("Y_firm");

v[4] = v[0] * v[1] / v[2] * v[3];

RESULT(v[4])


EQUATION("Y_firm")
/*
Output of firms
$Y_{i,j,t} = \frac{z_{i,j,t}}{z_{j,t}} Y_{j,t}$
*/

v[0] = V("z_firm");
v[1] = V("z_economy");
v[2] = V("Y_economy");

if (v[1] == 0 || isnan(v[0]) || isnan(v[1]) || isnan(v[2])) {
    v[3] = 0;  // or some other default value
} else {
    v[3] = v[0] / v[1] * v[2];
}

RESULT(v[3])



EQUATION("I_firm")
/*
Investment of firms
$I_{i,j,t} = \min\left\{ \iota_{i,j} Y_{i,j,t} ; \Pi_{i,j,t} \right\}$
*/

v[0] = V("iota_firm");
v[1] = V("Y_firm");
v[2] = V("Pi_firm");

v[3] = min(v[0] * v[1], v[2]);

RESULT(v[3])



EQUATION("R_firm")
/*
R&D expenses of firms
$R_{i,j,t} = \min \left\{ \rho_{i,j} Y_{i,j,t} ; \Pi_{i,j,t} - I_{i,j,t} \right\}$
*/

v[0] = V("rho_firm");
v[1] = V("Y_firm");
v[2] = V("Pi_firm");
v[3] = V("I_firm");
//v[5] = VL("w_economy", 1);

//v[4] = 1 / v[5] * min(v[0] * v[1], v[2] - v[3]);
v[4] = min(v[0] * v[1], v[2] - v[3]); // small change compared to the original model, the units do not make sense when using this to compute a probability otherwise

if (v[4] < 0) // to avoid problems numerical estimation problems
{
	v[4] = 0;
}

RESULT(v[4])



EQUATION("a_firm")
/*
Labour productivity embodied in the capital good developed by the firm during the previous period
Depends on innovation outcomes, which are modelled by a stochastic process
*/

v[0] = V("R_firm");
v[1] = V("Y_firm");
v[2] = VL("a_firm", 1);

v[3] = bernoulli(v[0] / v[1]); // equivalent and more simple than the uniform draw and then checking its value process presented in the paper

if (v[3]) // the R&D succeeded
{
	v[7] = V("innovator_firm");
	
	if (v[7]) // the firm is an innovator
	{
		v[5] = V("sigma_economy");
		
		v[6] = norm(0, v[5]);
	}
	else // the firm is an imitator
	{
		v[8] = V("chi_economy");
		v[9] = V("a_global");
		
		v[5] = max(v[8] * (v[9] - v[2]), 0); // added max by following @lorentz2018 [p. 189]
		
		if (v[5] == 0) // to avoid problems numerical estimation problems
		{
			v[6] = 0;
		}
		else
		{
			v[6] = norm(0, v[5]);
		}
	}
	
	v[4] = max(v[2], v[2] + v[6]);
}
else // the R&D failed
{
	v[4] = v[2];
}

RESULT(v[4])





// AGGREGATES AT THE ECONOMY LEVEL

EQUATION("z_economy")
/*
market share of the economy in the global economy
$z_{j,t} = \sum_i z_{i,j,t}$
*/

v[0] = SUM("z_firm");

RESULT(v[0])


EQUATION("z_economy")
/*
market share of the economy in the global economy
$z_{j,t} = \sum_i z_{i,j,t}$
*/

v[0] = SUM("z_firm");

RESULT(v[0])



EQUATION("E_economy")
/*
Competitiveness of economies
$E_{j,t} = \frac{1}{z_{j,t - 1}} \sum_i z_{i,j,t - 1} E_{i,j,t}$
*/

v[0] = VL("z_economy", 1);
v[3] = 0;

CYCLE(cur, "Firms")
{
    v[1] = VLS(cur, "z_firm", 1);
    v[2] = VS(cur, "E_firm");
    
    v[3] += v[1] * v[2];
}

if (v[0] == 0 || isnan(v[0]) || isnan(v[3])) {
    v[3] = 0;  // or some other default value
} else {
    v[3] /= v[0];
}

RESULT(v[3])



EQUATION("A_economy")
/*
Labour productivity of economies
$A_{j,t} = \frac{1}{z_{j,t - 1}} \sum_i z_{i,j,t - 1} A_{i,j,t}$
*/

v[0] = VL("z_economy", 1);
v[1] = 0;

CYCLE(cur, "Firms")
{
    v[2] = VLS(cur, "z_firm", 1);
    v[3] = VS(cur, "A_firm");
    
    v[1] += v[2] * v[3];
}

if (v[0] == 0 || isnan(v[0]) || isnan(v[1])) {
    v[1] = 0;  // or some other default value
} else {
    v[1] /= v[0];
}

RESULT(v[1])


EQUATION("p_economy")
/*
Average price in the economy
$\overline{p}_{j,t} = \frac{\sum_{i} p_{i,j,t}}{20}$
*/

v[0] = AVE("p_firm") ;

RESULT(v[0])






// MACRO PART: ECONOMY LEVEL


EQUATION("Firm_Exit_Entry")
/*
Handles the exit and entry of firms based on their market share.
*/

// Calculate the average market share in the economy
v[0] = AVE("z_firm");

// Calculate the average technological variables in the economy
v[7] = AVE("A_firm");
v[8] = AVE("E_firm");
v[9] = AVE("p_firm");
v[10] = AVE("K_firm");

// Initialize counters for exiting innovators and imitators
v[1] = 0; // Exiting Innovators
v[2] = 0; // Exiting Imitators

// Loop through all firms to check for exit conditions
CYCLE(cur, "Firms")
{
    v[3] = VS(cur, "z_firm");
    v[4] = VS(cur, "innovator_firm");
    
    // Check if the firm's market share is below the average
    if (v[3] < v[0])
    {
        // Count the type of exiting firm
        if (v[4] == 1)
        {
            v[1]++;
        }
        else
        {
            v[2]++;
        }
        
        // Replace the technological variables of the firm
        WRITES(cur, "z_firm", v[0]);
        WRITES(cur, "A_firm", v[7]);
        WRITES(cur, "E_firm", v[8]);
        WRITES(cur, "p_firm", v[9]);
        WRITES(cur, "K_firm", v[10]);
    }
}

RESULT(1)




EQUATION("l_economy")
/*
Employment rate in the economy
\[l_{j,t} = l_{j,t-1}\left(1+\frac{\Delta Y_{j,t}}{Y_{j,t-1}}\right)\left(1-\frac{\Delta A_{j,t}}{A_{j,t-1}}\right)\left(\frac{1}{1+n}\right)\]
*/
// Original calculation of l_{j,t} (let's call this v[6])
v[0] = VL("l_economy", 1);
v[1] = VL("Y_economy", 0);
v[2] = VL("Y_economy", 1);
v[3] = VL("A_economy", 0);
v[4] = VL("A_economy", 1);
v[5] = V("n_global");

if (v[2] == 0 || v[4] == 0 || isnan(v[0]) || isnan(v[1]) || isnan(v[2]) || isnan(v[3]) || isnan(v[4]) || isnan(v[5])) {
    v[6] = 0;  // or some other default value
} else {
    v[6] = v[0] * (1 + (v[1] / v[2] - 1)) * (1 - (v[3] / v[4] - 1)) * (1 / (1 + v[5])) ;
}

// Apply logistic function to limit the range of l_{j,t}
v[7] = 1 / (1 + exp(-v[6]));

RESULT(v[7])


EQUATION("w_economy")
/*
Wage dynamics in the economy
*/
v[0] = VL("w_economy", 1);
v[1] = V("gamma_economy");
v[2] = V("A_economy");
v[3] = VL("A_economy", 1);
v[4] = V("z_economy");
v[5] = V("p_economy");
v[6] = VL("p_economy", 1);
v[7] = V("l_economy");
v[8] = V("nu_0_economy") + V("nu_1_economy") * v[7] ;

if (v[3] == 0 || isnan(v[3]) || v[6] == 0 || isnan(v[6])) {
    v[9] = 0;  // or some other default value
} else {
    v[9] = v[0] * (1 + v[1] * (v[2] / v[3] - 1)) * (1 + v[4] * (v[5] / v[6] - 1)) * (1 + v[8]) ;
}

RESULT(v[9])




EQUATION("gamma_economy")
/*
Endogenized gamma variable using logistic function
\[\gamma_{j,t} = \sigma(\theta(\nu_{0} + \nu_{1}*l_{j,t}))\]
\[\sigma_{x} = \frac{1}{1+e^{-x}}\]
*/

v[0] = V("theta_global");  // theta parameter
v[1] = V("nu_0_economy");          // nu_0 parameter
v[2] = V("nu_1_economy");          // nu_1 parameter
v[3] = V("l_economy");     // l_{j,t} employment rate

// Calculate the argument for the logistic function
v[4] = v[0] * (v[1] + v[2] * v[3]);

// Apply the logistic function
v[5] = 1 / (1 + exp(-v[4]));

RESULT(v[5])



EQUATION("mu_economy")
/*
Markup dynamics in the economy
\[ \mu_{j,t} = \mu_{j,t-1} \left( 1+\xi(\frac{1}{1+\phi} - \gamma + z_{j,t-1}) \right) \]
*/
v[0] = VL("mu_economy", 1);
v[1] = V("xi_global");
v[2] = V("phi_global");
v[3] = V("gamma_economy");
v[4] = VL("z_economy", 1);

v[5] = v[0] * (1 + v[1] * (1 / (1 + v[2]) - v[3] + v[4]));

RESULT(v[5])


EQUATION("Yw_economy")
/*
GDP/demand of the RoW (for an economy $j$)
$Y_{w,t} = \sum_{j' \neq j} Y_{j',t} + Y_t^{exo}$
*/

v[0] = 0;
v[2] = V("Y_exo");

CYCLES(PARENT, cur, "Economies")
{
	if (cur != THIS)
	{
		v[1] = VS(cur, "Y_economy");
		
		v[0] += v[1];
	}
}

v[0] += v[2];

RESULT(v[0])



EQUATION("Y_economy")
/*
GDP = income = aggregate demand of an economy
$\hat{Y}_{j,t} = \frac{\alpha_j}{\beta_j} \hat{Y}_{w,t - 1} + \phi \frac{e_{j,t - 1}}{\beta_j} \left( \frac{E_{j,t}}{\bar{E}_t} - 1 \right)$
$e_{j,t} = \frac{1}{1 - z_{j,t}}$
*/

v[0] = VL("Y_economy", 1);
v[1] = V("alpha_economy");
v[2] = V("beta_economy");
v[3] = VL("Yw_economy", 1); // additionnal lag, like indicated in footnote 1, p. 1195
v[4] = VL("Yw_economy", 2);
v[5] = VL("z_economy", 1);
v[6] = V("E_economy");
v[7] = V("E_global");
v[9] = V("phi_global");

if (v[5] < 1) // to avoid problems numerical estimation problems
{
	v[8] = v[0] * (1 + v[1] / v[2] * (v[3] / v[4] - 1) + v[9] / v[2] / (1 - v[5]) * (v[6] / v[7] - 1));
}
else
{
	v[8] = v[0] * (1 + v[1] / v[2] * (v[3] / v[4] - 1));
}

RESULT(v[8])



EQUATION("GrowthY_economy")
/*
GDP/demand growth of the economy
used as indicator when analysing the model, no effect on the dynamics
$\hat{Y}_{j, t}$
*/

v[0] = V("Y_economy");
v[1] = VL("Y_economy", 1);

if (v[1] == 0 || v[0] == 0) // to avoid problems numerical estimation problems
{
	v[2] = 0;
}
else
{
	v[2] = v[0] / v[1] - 1;
}

RESULT(v[2])



EQUATION("GrowthA_economy")
/*
Productivity growth of the economy
used as indicator when analysing the model, no effect on the dynamics
$\hat{A}_{j, t}$
*/

v[0] = V("A_economy");
v[1] = VL("A_economy", 1);

if (v[1] == 0 || v[0] == 0) // to avoid problems numerical estimation problems
{
	v[2] = 0;
}
else
{
	v[2] = v[0] / v[1] - 1;
}

RESULT(v[2])









// AGGREGATES AT THE GLOBAL LEVEL



EQUATION("E_global")
/*
Average competitiveness of the global market
$\bar{E}_t = \sum_j z_{j,t - 1} E_{j,t}$
*/

v[0] = 0;

CYCLE(cur, "Economies")
{
	v[1] = VS(cur, "E_economy");
	v[2] = VLS(cur, "z_economy", 1);
	
	v[0] += v[1] * v[2];
}

RESULT(v[0])



EQUATION("a_global")
/*
Average productivity level embodied in the latest capital vintages developed by firms
$\bar{a}_t = \sum_{j,i} z_{i,j,t} a_{i,j,t - 1}$
*/

v[0] = 0;

CYCLE(cur1, "Economies")
{
	CYCLES(cur1, cur2, "Firms")
	{
		v[1] = VS(cur2, "z_firm");
		v[2] = VLS(cur2, "a_firm", 1);
		
		v[0] += v[1] * v[2];
	}
}


RESULT(v[0])



EQUATION("CoefVarGrowthY_global")
/*
Coefficient of variation of GDP growth rates among economies
used as indicator when analysing the model, no effect on the dynamics
Coefficient of variation = Standard Deviation / Mean
*/

v[0] = SD("GrowthY_economy");
v[1] = AVE("GrowthY_economy");

if (v[1] == 0 || is_nan(v[0]))
{
	v[2] = 0;
}
else
{
	v[2] = v[0] / v[1];
}

RESULT(v[2])



EQUATION("CoefVarGrowthA_global")
/*
Coefficient of variation of productivity growth rates among economies
used as indicator when analysing the model, no effect on the dynamics
Coefficient of variation = Standard Deviation / Mean
*/

v[0] = SD("GrowthA_economy");
v[1] = AVE("GrowthA_economy");

if (v[1] == 0 || is_nan(v[0]))
{
	v[2] = 0;
}
else
{
	v[2] = v[0] / v[1];
}

RESULT(v[2])






// ECONOMIES OUTSIDE THE MODELLED ONES



EQUATION("Y_exo")
/*
exogenous component of external demand/GDP
$Y_t^{exo} = (1 + \lambda^{exo}) Y_{t - 1}^{exo}$
*/

v[0] = VL("Y_exo", 1);
v[1] = V("lambda_exo");

v[2] = (1 + v[1]) * v[0];

RESULT(v[2])





MODELEND

// do not add Equations in this area

void close_sim( void )
{
	// close simulation special commands go here
}
