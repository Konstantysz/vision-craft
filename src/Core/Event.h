#pragma once

namespace Core
{
	/**
	 * @brief Base class for all events in the application.
	 */
	class Event
	{
	public:
		virtual ~Event() = default;
	};
} // namespace Core