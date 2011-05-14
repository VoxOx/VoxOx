/*  This file is not of much use. It is added to workarround a bug in visual c++
 *  which complains about unresolved external symbol __mb_cur_max
 *
 */
#ifdef _WIN32
extern "C" {
	int __mb_cur_max;
	int errno;
}
#endif
