#pragma once
#include "../Common/NonCopyable.h"
#include "ApiVersion.h"

namespace TVPlayR {
	namespace Decklink {

class Decklink;

class Iterator: Common::NonCopyable
{
public:
	Iterator();
	~Iterator(); 
	std::shared_ptr<Decklink> operator [] (size_t pos);
	size_t Size() const;
	ApiVersion GetVersion();
	void Refresh();
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}