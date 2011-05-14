#ifndef OWPL_H_
#define OWPL_H_

typedef int OWPL_CALL;
typedef int OWPL_SUB;
typedef int OWPL_LINE;
typedef int OWPL_PUB;

/*
 * OWPL_RESULT: The different results that can be returned from a function in phAPI.h
 */
typedef enum OWPL_RESULT 
{
    OWPL_RESULT_SUCCESS = 0,         /**< Success */
    OWPL_RESULT_FAILURE,             /**< Generic Failure*/
    OWPL_RESULT_NOT_IMPLEMENTED,     /**< Method/API not implemented */
    OWPL_RESULT_OUT_OF_MEMORY,       /**< Unable to allocate enough memory to perform operation*/
    OWPL_RESULT_INVALID_ARGS,        /**< Invalid arguments; bad handle, argument out of range, 
                                          etc.*/
    OWPL_RESULT_BAD_ADDRESS,         /**< Invalid SIP address */
    OWPL_RESULT_OUT_OF_RESOURCES,    /**< Out of resources (hit some max limit) */
    OWPL_RESULT_INSUFFICIENT_BUFFER, /**< Buffer too short for this operation */
    OWPL_RESULT_BUSY,                /**< The operation failed because the system was busy */
    OWPL_RESULT_INVALID_STATE,       /**< The operation failed because the object was in
                                          the wrong state.  For example, attempting to split
                                          a call from a conference before that call is 
                                          connected. */
} OWPL_RESULT ;

#endif /* OWPL_H_ */
