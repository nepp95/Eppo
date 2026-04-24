#pragma once

namespace Eppo
{
	class UUID
	{
	public:
		UUID();
		~UUID() = default;

		operator bool() const { return m_UUID != 0; }

	private:
		uint64_t m_UUID;
	};
}

template<>
struct std::hash<Eppo::UUID>
{
	auto operator()(const Eppo::UUID& uuid) const noexcept -> std::size_t
	{
		return hash<uint64_t>()(static_cast<uint64_t>(uuid));
	}
};