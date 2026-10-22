#pragma once
#include <string>
#include <vector>

#include <atomic>
#include <memory>

#include "strategies/IRenderStrategy.h"
#include "PerformanceMonitor.h"
#include "core/IScreenCapture.h"
class CoreEngine{

    protected:
        bool m_measure_performance;             ///< flag that indicates whether to log measured performance or not
        double m_targetFPS=0;
        std::atomic<bool> m_isEngineRunning;      ///< Atomic flag indicating if the engine is currently active.

        PerformanceMonitor m_Pmonitor;
        std::unique_ptr<IRenderStrategy> m_currentStrategy; ///< Exclusively owned active rendering strategy.
        std::vector<Property> m_activeProperties;///< List of adjustable properties for the active strategy.
        int m_selectedPropertyIndex = 0;         ///< Index of the currently highlighted property in the HUD.
        int m_menuStartIndex = 0;                ///< Index of the first visible property in the HUD (for scrolling).

        /**
        * @brief Set the Strategy object to be used for rendering frames.
        *  using unique_ptr for automatic memory management.
        *  Also updates the active properties list for the HUD.
        * 
        * @param newStrategy Name of the new strategy to switch to. The StrategiesFactory will create the appropriate object.
        */
        void setStrategy(std::string newStrategy);

        virtual void play() = 0;
        virtual bool init() = 0;
};