#ifndef LIMITER_H 
#define LIMITER_H 

// cut value direct to limit

template <class T>
void cut_scope (T& value, T& min, T& max)
{
	if      (value < min) value = min;
	else if (value > max) value = max;
}

template <class T>
void cut_inside (T& value, T& var1, T& var2)
{
	if (var1 < var2) cut_scope (value, var1, var2);
	else             cut_scope (value, var2, var1);
}

template <class T>
void cut_min (T& value, T& min)
{
	if (value < min) value = min;
}
template <class T>
void cut_min (T& value, T& min1, T& min2)
{
	if (value < min1) value = min1;
	if (value < min2) value = min2;
}

template <class T>
void cut_max (T& value, T& max)
{
	if (value > max) value = max;
}
template <class T>
void cut_max (T& value, T& max1, T& max2)
{
	if (value > max1) value = max1;
	if (value > max2) value = max2;
}

// templates for different types
template <class T, class V1, class V2>
inline void cut_scope (T& value, V1 min, V2 max)  {cut_scope (value, (T) min, (T) max);}

template <class T, class V1, class V2>
inline void cut_inside (T& value, V1 var1, V2 var2)  {cut_inside (value, (T) var1, (T) var2);}

template <class T, class V1>
inline void cut_min (T& value, V1 min)  {cut_min (value, (T) min);}
template <class T, class V1, class V2>
inline void cut_min (T& value, V1 min1, V2 min2)  {cut_min (value, (T) min1, (T) min2);}

template <class T, class V1>
inline void cut_max (T& value, V1 max)  {cut_max (value, (T) max);}
template <class T, class V1, class V2>
inline void cut_max (T& value, V1 max1, V2 max2)  {cut_max (value, (T) max1, (T) max2);}

// get limited value

template <class T>
T get_cut_scope (T& value, T& min, T& max)
{
	if      (value < min) return min;
	else if (value > max) return max;
	return value;
}

template <class T>
T get_cut_inside (T& value, T& var1, T& var2)
{
	if (var1 < var2) return get_cut_scope (value, var1, var2);
	else             return get_cut_scope (value, var2, var1);
}

template <class T>
T get_cut_min (T& value, T& min)
{
	if (value < min) return min;
	return value;
}
template <class T>
T get_cut_min (T& value, T& min1, T& min2)
{
	if (min1 > min2)
	{
		if (value < min1) return min1;
		if (value < min2) return min2;
	}
	else
	{
		if (value < min2) return min2;
		if (value < min1) return min1;
	}
	return value;
}

template <class T>
T get_cut_max (T& value, T& max)
{
	if (value > max) return max;
	return value;
}
template <class T>
T get_cut_max (T& value, T& max1, T& max2)
{
	if (max1 < max2)
	{
		if (value > max1) return max1;
		if (value > max2) return max2;
	}
	else
	{
		if (value > max2) return max2;
		if (value > max1) return max1;
	}
	return value;
}

// templates for different types
template <class T, class V1, class V2>
inline T get_cut_scope (T& value, V1 min, V2 max)  {return get_cut_scope (value, (T) min, (T) max);}

template <class T, class V1, class V2>
inline T get_cut_inside (T& value, V1 var1, V2 var2)  {return get_cut_inside (value, (T) var1, (T) var2);}

template <class T, class V1>
inline T get_cut_min (T& value, V1 min)  {return get_cut_min (value, (T) min);}
template <class T, class V1, class V2>
inline T get_cut_min (T& value, V1 min1, V2 min2)  {return get_cut_min (value, (T) min1, (T) min2);}

template <class T, class V1>
inline T get_cut_max (T& value, V1 max)  {return get_cut_max (value, (T) max);}
template <class T, class V1, class V2>
inline T get_cut_max (T& value, V1 max1, V2 max2)  {return get_cut_max (value, (T) max1, (T) max2);}

// check, return true if is in limit

template <class T>
bool is_cut_scope (T& value, T& min, T& max)
{
	if      (value < min) return false;
	else if (value > max) return false;
	return true;
}

template <class T>
bool is_cut_inside (T& value, T& var1, T& var2)
{
	if (var1 < var2) return is_cut_scope (value, var1, var2);
	else             return is_cut_scope (value, var2, var1);
}

template <class T>
bool is_cut_min (T& value, T& min)
{
	if (value < min) return false;
	return true;
}
template <class T>
bool is_cut_min (T& value, T& min1, T& min2)
{
	if (min1 > min2)
	{
		if (value < min1) return false;
		if (value < min2) return false;
	}
	else
	{
		if (value < min2) return false;
		if (value < min1) return false;
	}
	return true;
}

template <class T>
bool is_cut_max (T& value, T& max)
{
	if (value > max) return false;
	return true;
}
template <class T>
bool is_cut_max (T& value, T& max1, T& max2)
{
	if (max1 < max2)
	{
		if (value > max1) return false;
		if (value > max2) return false;
	}
	else
	{
		if (value > max2) return false;
		if (value > max1) return false;
	}
	return true;
}

// templates for different types
template <class T, class V1, class V2>
inline bool is_cut_scope (T& value, V1 min, V2 max)  {return is_cut_scope (value, (T) min, (T) max);}

template <class T, class V1, class V2>
inline bool is_cut_inside (T& value, V1 var1, V2 var2)  {return is_cut_inside (value, (T) var1, (T) var2);}

template <class T, class V1>
inline bool is_cut_min (T& value, V1 min)  {return is_cut_min (value, (T) min);}
template <class T, class V1, class V2>
inline bool is_cut_min (T& value, V1 min1, V2 min2)  {return is_cut_min (value, (T) min1, (T) min2);}

template <class T, class V1>
inline bool is_cut_max (T& value, V1 max)  {return is_cut_max (value, (T) max);}
template <class T, class V1, class V2>
inline bool is_cut_max (T& value, V1 max1, V2 max2)  {return is_cut_max (value, (T) max1, (T) max2);}

// cut value direct to limit, return true if it was limit

template <class T>
bool was_cut_scope (T& value, T& min, T& max)
{
	if      (value < min) {value = min; return false;}
	else if (value > max) {value = max; return false;}
	return true;
}

template <class T>
bool was_cut_inside (T& value, T& var1, T& var2)
{
	if (var1 < var2) return was_cut_scope (value, var1, var2);
	else             return was_cut_scope (value, var2, var1);
}

template <class T>
bool was_cut_min (T& value, T& min)
{
	if (value < min) {value = min; return false;}
	return true;
}
template <class T>
bool was_cut_min (T& value, T& min1, T& min2)
{
	bool is_unchanged = true;
	if (value < min1) {value = min1; is_unchanged = false;}
	if (value < min2) {value = min2; return false;}
	return is_unchanged;
}

template <class T>
bool was_cut_max (T& value, T& max)
{
	if (value > max) {value = max; return false;}
	return true;
}
template <class T>
bool was_cut_max (T& value, T& max1, T& max2)
{
	bool is_unchanged = true;
	if (value > max1) {value = max1; is_unchanged = false;}
	if (value > max2) {value = max2; return false;}
	return is_unchanged;
}

// templates for different types
template <class T, class V1, class V2>
inline bool was_cut_scope (T& value, V1 min, V2 max)  {return was_cut_scope (value, (T) min, (T) max);}

template <class T, class V1, class V2>
inline bool was_cut_inside (T& value, V1 var1, V2 var2)  {return was_cut_inside (value, (T) var1, (T) var2);}

template <class T, class V1>
inline bool was_cut_min (T& value, V1 min)  {return was_cut_min (value, (T) min);}
template <class T, class V1, class V2>
inline bool was_cut_min (T& value, V1 min1, V2 min2)  {return was_cut_min (value, (T) min1, (T) min2);}

template <class T, class V1>
inline bool was_cut_max (T& value, V1 max)  {return was_cut_max (value, (T) max);}
template <class T, class V1, class V2>
inline bool was_cut_max (T& value, V1 max1, V2 max2)  {return was_cut_max (value, (T) max1, (T) max2);}


#endif // LIMITER_H 
