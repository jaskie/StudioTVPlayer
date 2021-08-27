#pragma once
#include "../Common/NonCopyable.h"
#include "ApiVersion.h"

namespace TVPlayR {
	namespace Decklink {

class DecklinkOutput;
class DecklinkInfo;

class DecklinkIterator: Common::NonCopyable
{
public:
	DecklinkIterator();
	~DecklinkIterator(); 
	std::shared_ptr<DecklinkInfo> operator [] (size_t pos);
	std::shared_ptr<DecklinkOutput> CreateOutput(int index);
	size_t Size() const;
	std::shared_ptr<ApiVersion> GetVersion();
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}