//Telcentris copyright

#ifndef ENUM_VISIBILITY_H
#define ENUM_VISIBILITY_H


//#include <util/NonCopyable.h>

#include <string>

//JRT - 2009.03.25 - These definitions are preliminary and may be expanded.
/**
 * Visibility: public    - visible to all
 *			   private   - Visible only to owner
 *			   protected - Visible to members of owners contact list.
 *
 * @author Jeff Theinert
 */
class EnumVisibility //: NonCopyable {
{
public:

	enum Visibility 
	{
		VisibilityUnknown,
		VisibilityPrivate,
		VisibilityProtected,
		VisibilityPublic,
	};

	/**
	 * Gets a string represencation of the visibility.
	 *
	 * @param sex - the visibility to transform
	 * @result - the string representing the visibility
	 */
	static std::string toString(Visibility vis);

	/**
	 * Gets the visibility from a string representation.
	 *
	 * @param sex - the string to transform.
	 * @return  - the visibility represented by the string
	 */
	static Visibility toVisibility(const std::string & vis);
};

#endif //ENUM_VISIBILITY_H
