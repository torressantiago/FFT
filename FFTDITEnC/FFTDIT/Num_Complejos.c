#include "Num_Complejos.h"

COMPLEX ConversionCart2Pol	(COMPLEX num){
	COMPLEX res;
	res.m = sqrt(pow(num.x,2)+pow(num.y,2));
	res.a = atan((num.y)/(num.x));
	res.x = num.x;
	res.y = num.y;
	return res;
}

COMPLEX ConversionPol2Cart	(COMPLEX num){
	COMPLEX res;
	res.x = cos(num.a)*num.m;
	res.y = sin(num.a)*num.m;
	res.m = num.m;
	res.a = num.a;
	return res;
}

COMPLEX ComplexSuma			(COMPLEX num1, COMPLEX num2){
	COMPLEX res;
	res.x = num1.x + num2.x;
	res.y = num1.y + num2.y;
    return ConversionCart2Pol(res);
}

COMPLEX ComplexResta		(COMPLEX num1, COMPLEX num2){
	COMPLEX res;
	res.x = num1.x - num2.x;
	res.y = num1.y - num2.y;
    return ConversionCart2Pol(res);
}

COMPLEX ComplexMult			(COMPLEX num1, COMPLEX num2){
	COMPLEX res;
	res.m = num1.m * num2.m;
	res.a = num1.a + num2.a;
    return ConversionPol2Cart(res);
}

COMPLEX WnCalculator		(double rval){
	COMPLEX res;
	res.m = 1;
	res.a = -2*M_PI*rval/NVAL;
	return (ConversionPol2Cart(res));
}
