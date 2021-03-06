#if defined(UUID_FUN_NOTE)
	/*
	Description
	The uuid_clear function sets the value of the supplied uuid variable
	uu to the NULL value.
	*/
	void uuid_clear(uuid_t uu);//uuid置空

	/*
	Description
	The uuid_compare function compares the two supplied uuid variables uu1
	and uu2 to each other.

	Return Value
	Returns an integer less than, equal to, or greater than zero if uu1 is
	found, respectively, to be lexigraphically less than, equal, or greater than uu2.
	*/
	int uuid_compare(uuid_t uu1, uuid_t uu2);

	/*
	Description
	The uuid_copy function copies the UUID variable src to dst.
	Return Value
	The copied UUID is returned in the location pointed to by dst.
	*/
	void uuid_copy(uuid_t dst, uuid_t src);

	/*
	Description

	The uuid_generate function creates a new universally unique identifier (UUID). 
	The uuid will be generated based on high-quality randomness from /dev/urandom, 
	if available. If it is not available, then uuid_generate will use an alternative 
	algorithm which uses the current time, the local ethernet MAC address (if available),
	and random data generated using a pseudo-random generator.

	The uuid_generate_random function forces the use of the all-random UUID format,
	even if a high-quality random number generator (i.e., /dev/urandom) is not available, 
	in which case a pseudo-random generator will be substituted. Note that the use of a 
	pseudo-random generator may compromise the uniqueness of UUIDs generated in this fashion.
	The uuid_generate_time function forces the use of the alternative algorithm which uses
	the current time and the local ethernet MAC address (if available). This algorithm used
	to be the default one used to generate UUID, but because of the use of the ethernet MAC
	address, it can leak information about when and where the UUID was generated. This can 
	cause privacy problems in some applications, so the uuid_generate function only uses this
	algorithm if a high-quality source of randomness is not available. To guarantee uniqueness
	of UUIDs generated by concurrently running processes, the uuid library uses global clock 
	state counter (if the process has permissions to gain exclusive access to this file) and/or
	the uuidd daemon, if it is running already or can be be spawned by the process (if installed
	and the process has enough permissions to run it). If neither of these two synchronization
	mechanisms can be used, it is theoretically possible that two concurrently running processes
	obtain the same UUID(s). To tell whether the UUID has been generated in a safe manner, use
	uuid_generate_time_safe.

	The uuid_generate_time_safe is similar to uuid_generate_time, except that it returns a value
	which denotes whether any of the synchronization mechanisms (see above) has been used.

	The UUID is 16 bytes (128 bits) long, which gives approximately 3.4x10^38 unique values
	(there are approximately 10^80 elementary particles in the universe according to Carl Sagan's
	Cosmos). The new UUID can reasonably be considered unique among all UUIDs created on the local
	system, and among UUIDs created on other systems in the past and in the future.

	Return Value
	The newly created UUID is returned in the memory location pointed to by out. 
	uuid_generate_time_safe returns zero if the UUID has been generated in a safe manner,
	-1 otherwise.
	*/
	void uuid_generate(uuid_t out);
	void uuid_generate_random(uuid_t out);
	void uuid_generate_time(uuid_t out);
	int uuid_generate_time_safe(uuid_t out);

	/*
	Description
	The uuid_is_null function compares the value of the supplied UUID variable uu to the NULL
	value. If the value is equal to the NULL UUID, 1 is returned, otherwise 0 is returned.
	*/
	int uuid_is_null(uuid_t uu);//直接定义后的uuid不为空，用uuid_clear初始化后为空，此时返回1

	/*
	Description
	The uuid_parse function converts the UUID string given by in into the binary representation.
	The input UUID is a string of the form 1b4e28ba-2fa1-11d2-883f-b9a761bde3fb (in printf(3) 
	format "%08x-%04x-%04x-%04x-%012x", 36 bytes plus the trailing '\0').
	Return Value
	Upon successfully parsing the input string, 0 is returned, and the UUID is stored in the 
	location pointed to by uu, otherwise -1 is returned.
	*/
	int uuid_parse( char *in, uuid_t uu);

	/*
	Description
	The uuid_time function extracts the time at which the supplied time-based UUID uu was 
	created. Note that the UUID creation time is only encoded within certain types of UUIDs.
	This function can only reasonably expect to extract the creation time for UUIDs created 
	with the uuid_generate_time(3) and uuid_generate_time_safe(3) functions. It may or may 
	not work with UUIDs created by other mechanisms.
	Return Values
	The time at which the UUID was created, in seconds since January 1, 1970 GMT (the epoch), 
	is returned (see time(2)). The time at which the UUID was created, in seconds and 
	microseconds since the epoch, is also stored in the location pointed to by ret_tv 
	(see gettimeofday(2)).
	*/
	time_t uuid_time(uuid_t uu, struct timeval *ret_tv);

	/*
	Description

	The uuid_unparse function converts the supplied UUID uu from the binary representation
	into a 36-byte string (plus tailing '\0') of the form 1b4e28ba-2fa1-11d2-883f-0016d3cca427
	and stores this value in the character string pointed to by out. The case of the hex digits
	returned by uuid_unparse may be upper or lower case, and is dependent on the system-dependent
	local default.
	 
	If the case of the hex digits is important then the functions uuid_unparse_upper and 
	uuid_unparse_lower may be used.

	*/
	void uuid_unparse(uuid_t uu, char *out);//默认小写，把uuid转化为字符指针
	void uuid_unparse_upper(uuid_t uu, char *out);
	void uuid_unparse_lower(uuid_t uu, char *out);
#endif

