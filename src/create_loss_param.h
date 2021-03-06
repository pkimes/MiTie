#ifndef _CREATE_LOSS_PARAM_H__
#define _CREATE_LOSS_PARAM_H__

#include "math_tools.h"
double compute_KL(double (&w)[2], double X[][2], double Y[], vector<int> idx)
{
	//compute Kullback-Leiber divergence
	//
	// \sum_x ln(\frac{p(x)}{q(x)})*p(x)
	//
	// using log(a+b) = log(a) + log(1+exp(log(b)-log(a)))
	double n = -X[idx[0]][0]*w[0]-X[idx[0]][1]*w[1];
	for (int i=1; i<idx.size(); i++)
	{
		double val = -X[idx[i]][0]*w[0]-X[idx[i]][1]*w[1]; 
		n += log(1+exp(val-n)); 
	}

	double m = -Y[idx[0]];
	for (int i=1; i<idx.size(); i++)
	{
		double val = -Y[idx[i]];
		m+= log(1+exp(val-m)); // normalize
	}

	double ret = 0;
	for (int i=0; i<idx.size(); i++) 
	{
		double log_val1 = -Y[idx[i]]-m;
		double log_val2 = -X[idx[i]][0]*w[0]-X[idx[i]][1]*w[1]-n;
		ret += (log_val1-log_val2)*exp(log_val1);
	}
	return ret;
}


double compute_bhatt(double (&w)[2], double X[][2], double Y[], vector<int> idx)
{
	//compute Bhattacharyya coefficient
	//
	// \sum_x \sqrt{p(x)*q(x)}
	//
	// using log(a+b) = log(a) + log(1+exp(log(b)-log(a)))
	double n = -X[idx[0]][0]*w[0]-X[idx[0]][1]*w[1];
	for (int i=1; i<idx.size(); i++)
	{
		double val = -X[idx[i]][0]*w[0]-X[idx[i]][1]*w[1]; 
		n += log(1+exp(val-n)); 
	}

	double m = -Y[idx[0]];
	for (int i=1; i<idx.size(); i++)
	{
		double val = -Y[idx[i]];
		m+= log(1+exp(val-m)); // normalize
	}

	double ret = 0;
	for (int i=0; i<idx.size(); i++) 
	{
		double val1 = exp(-Y[idx[i]]-m);
		double val2 = exp(-X[idx[i]][0]*w[0]-X[idx[i]][1]*w[1]-n);
		ret += -sqrt(val1*val2);
	}
	return ret;
}


bool my_min(double (&w)[2], double X[][2], double Y[], vector<int> idx, double box11, double box12, double box21, double box22)
{
	double eps = 1e-10;
	w[0] = box11; 
	w[1] = box21;

	//double best = compute_bhatt(w, X, Y, idx); 
	double best = compute_KL(w, X, Y, idx); 
	double best_orig = best;
	double w_best[2]; 
	w_best[0]= w[0];
	w_best[1]= w[1];

	int iter = 0;

	while (box12-box11>eps || box22-box21>eps)
	{
		best = best_orig;

		double step1 = (box12-box11)/10;
		double step2 = (box22-box21)/10;

		//printf("best:%.5f %.10f %.10f (%.10f) %.10f %.10f (%.10f)\n", best, box11, box12, step1, box21, box22, step2);
		for (int i=0; i<=10; i++)
		{
			double val1=box11+i*step1; 
			for (int j=0; j<=10; j++)
			{
				double val2 = box21+j*step2;
				w[0] = val1; 
				w[1] = val2;
				//double val = compute_bhatt(w, X, Y, idx);
				double val = compute_KL(w, X, Y, idx);
				if (val<best)
				{
					//printf("val:%.5f, best:%.5f\n", val, best);
					best = val;
					w_best[0] = w[0];
					w_best[1] = w[1];
				}
			}
		}	
		if (box12-box11>eps)
		{
			double step = (box12-box11)/10;
			for (int i=0; i<=10; i++)
			{
				double val1=box11+i*step; 
				if (w_best[0] == val1 && val1>box11 && val1<box12)
				{
					box11 = val1-step;
					box12 = val1+step;
					break;
				}
				else if (w_best[0] == val1 && val1==box11)
				{
					box12 = val1+step;
					break;
				}
				else if (w_best[0] == val1 && val1==box12)
				{
					box11 = val1-step;
					break;
				}
			}
		}
		if (box22-box21>eps)
		{
			double step = (box22-box21)/10;
			for (int j=0; j<=10; j++)
			{
				double val2 = box21+j*step;

				if (w_best[1] == val2 && val2>box21 && val2<box22)
				{
					box21 = val2-step;
					box22 = val2+step;
					break;
				}
				else if (w_best[1] == val2 && val2==box21)
				{
					box22 = val2+step;
					break;
				}
				else if (w_best[1] == val2 && val2==box22)
				{
					box21 = val2-step;
					break;
				}
			}
		}
		iter++;
		if (iter>100) // may happen for numerical reasons if eps too small
			break;
			
	}
	//printf("[%.15f, %.15f (%.15f)][%.15f, %.15f (%.15f)] %.6f, %.6f\n", box11, box12, box12-box11, box21, box22, box22-box21, w[0], w[1]);
	//printf("iter:%i\n", iter);
}


vector<vector<double> > create_loss_parameters(float eta1, float eta2, float lambda, int order)
{
	
	vector<vector<double> > ret;

	char fn_param[1000];
	sprintf(fn_param, "param/loss_param_eta1%.3f_eta2%.3f_lambda%.3f_order%i.txt", eta1, eta2, lambda, order);

	if (fexist(fn_param))
	{
		printf("parameter file exists\nload parameters from file:%s\n", fn_param);

		FILE* fd = fopen(fn_param, "r");
		while(~feof(fd))
		{
			char line[1000]; 
			if (fgets(line, 1000, fd)==NULL) break; 

			if (line[0]=='#')
				continue; 

			vector<char*> fields = get_fields(line);
			
			assert(fields.size()==5);
			vector<double> tmp; 
			for (int i=0; i<5; i++)
			{
				tmp.push_back(atof(fields[i]));
			}
			ret.push_back(tmp);
		}
		fclose(fd);
		return ret;
	}
	else
	{
		printf("compute loss param\n");
	}

	// order of the polynom to fit the loss function
	assert(order==1 || order==2);


	// this is the observed coverage:
	//int xpos[] = {1, 2, 3, 5, 10, 15, 20, 35, 50, 100, 200, 500, 1000, 5000, 10000, 30000};
	//int num_pos = sizeof(xpos)/sizeof(int);
	vector<float> xpos = logspace<float>(1.0, 5e4, 50);
	int num_pos = xpos.size();;
	//print_vec(&xpos2, "%.4f, ");

	double left_q[num_pos];
	double left_l[num_pos];
	double right_q[num_pos];
	double right_l[num_pos];
	
	for (int s=0; s<num_pos; s++)
	{
		float obs = xpos[s];
	
		float std_= sqrt(eta1*(obs+1) + pow(eta2*obs, 2));
		vector<float> mus;
		for (int j=-5; j<5; j++)
		{
			if  (obs+j*std_<1 && j<-1)
			{
				continue;
			}
			for (float f=obs+j*std_; f<obs+(j+1)*std_; f+=std_/100)
			{
				if (f>=1)
					mus.push_back(f);
			}
		}
		
		double Y[mus.size()];
		for (int k=1; k<mus.size(); k++)
		{
			float mu = mus[k];
	
			float var_ = eta1*mu + pow(eta2*mu, 2);
	
			// compute parameters of negative binomial
			double p = 1-(mu/var_);
			double r = mu*(1-p)/p;
	
			// A ~ Pois(lambda)
			// B ~ NB(r,p)
			// obs = A+B 
			//
			for (int i=0; i<100 && i<obs; i++)
			{
				double pois;
				if (lambda==0 && i==0)
				{
					pois = 1;
				}
				else if (lambda==0)
				{
					break;
				}
				else
				{
					pois = i*log(lambda) - lambda - factln(i); 
				}
	
				double nb;
				if (eta1==1 && eta2==0)
				{
					// special case: in the limit of var->mu we get the poisson distrib
					nb = (obs-i)*log(mu) - mu - factln(obs-i);
				}
				else
				{
					nb = factln(obs-i+r-1) - factln(obs-i) - factln(r-1) + r*log(1-p) + (obs-i)*log(p); 
				}
	
				// naive: Y(k) = Y(k) + exp(pois+nb); 
				// using log(a+b) = log(a) + log(1+exp(log(b)-log(a)))
				if (i==0)
				{
					Y[k] = -(nb+pois);
				}
				else
				{
					Y[k] = Y[k] - (log(1+exp(nb+pois-Y[k])));
				}
			}
		}	
	
		double X[mus.size()][2];
		vector<int> idx1; 
		vector<int> idx2; 
		for (int k=1; k<mus.size(); k++) 
		{
			X[k][0] = pow(mus[k]-obs, 2);
			X[k][1] = mus[k]-obs;
			if (mus[k]<=obs);
				idx1.push_back(k);
			if (mus[k]>=obs)
				idx2.push_back(k);
		}
	
			
		double offset;
		if (idx1.size()>0)
			offset = Y[idx1.back()];
		else
			offset=Y[idx2[0]];
	
		double w1[2];
		if (idx1.empty())
		{
			if (order==1)
				w1[0] = 0; 
			else
				w1[0] = 1e-10; 
			w1[1] = 1e-10;
		}
		else
		{
			if (order==1)
				my_min(w1, X, Y, idx1, 0, 1e-10, 1e-3, 10);
			else
				my_min(w1, X, Y, idx1, 1e-10, 0.5, 1e-10, 10);
		}
		assert(!idx2.empty());
		double w2[2];
		if (order==1)
			my_min(w2, X, Y, idx1, 0, 1e-10, 1e-3, 10);
		else
			my_min(w2, X, Y, idx1, 1e-10, 0.5, 1e-10, 10);
		
		
		left_l[s]=w1[1];
		left_q[s]=w1[0];
		right_l[s]=w2[1];
		right_q[s]=w2[0];
	
		vector<double> tmp;
		tmp.push_back(obs);
		tmp.push_back(w1[1]);// left_l
		tmp.push_back(w1[0]);// left_q
		tmp.push_back(w2[1]);// right_l
		tmp.push_back(w2[0]);// right_q
		ret.push_back(tmp);
	} 

	FILE* fd = fopen(fn_param, "w");
	if (!fd)
	{
		printf("could not open file %s for writing\n", fn_param);
		exit(0);
	}
	printf("##observation\tleft_l\tleft_q\tright_l\tright_q\n");
	for (int j=0; j<num_pos; j++)
	{
		fprintf(fd, "%.2f\t%.15f\t%.15f\t%.15f\t%.15f\n", xpos[j], left_l[j], left_q[j], right_l[j], right_q[j]);
	}
	fclose(fd);
	return ret;
}


#endif
