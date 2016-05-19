#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>

#define __MENSAGEM_BT4__ "BT4"
#define __MENSAGEM_INFOMODULO__ "INF"
#define __MENSAGEM_GPS__ "GPS"

namespace Sascar {

using namespace std;

class Message
{
	public:
		Message();
		virtual ~Message();

		inline void SetName(const string name){ sName = name; }
		inline const string &GetName() const { return sName; }

		inline void SetType(const string type){ sType = type; }
		inline const string &GetType() const { return sType; }

		inline void SetPlate(const string plate){ sPlate = plate; }
		inline const string &GetPlate() const { return sPlate; }

		inline void SetClient(const string client){ sClient = client; }
		inline const string &GetClient() const { return sClient; }

		inline void SetMessage(uint8_t *message){ pMessage = message; }
		inline const uint8_t *GetMessage() { return pMessage; }

		inline void SetMessageSize(const uint64_t messageSize){ iMessageSize = messageSize; }
		inline const uint64_t &GetMessageSize() const { return iMessageSize; }

		inline void SetUpdatedAt(const uint64_t updatedAt){ iUpdatedAt = updatedAt; }
		inline const uint64_t &GetUpdatedAt() const { return iUpdatedAt; }

		inline void SetSource(const string source){ sSource = source; }
		inline const string &GetSource() const { return sSource; }

	private:
		string sName;
		string sType;
		string sPlate;
		string sClient;
		uint8_t *pMessage;
		int iMessageSize;
		uint64_t iUpdatedAt;
		string sSource;
};

}

#endif // MESSAGE_HPP

