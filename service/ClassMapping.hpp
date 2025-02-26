#include "../Main.hpp"

namespace IL2CPP
{
	namespace ClassMapping
	{
		void AddImageToScan(Image* image);

		void AddImageToScan(const std::string& assemblyName);

		void AddQueue(const std::string& mapKeyname, const std::string& namespaze, const FieldPattern* pattern);

		void AddQueue(const std::string& mapKeyname, const std::string& namespaze);

		void ClearImagesToScan();

		void ClearMapping();

		void ClearQueue();

		void StartMapping();

		Class* GetClass(const std::string& mapKeyname);
	}
}