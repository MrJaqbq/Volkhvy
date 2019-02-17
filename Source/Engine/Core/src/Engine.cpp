//
// Created by mrjaqbq on 07.03.16.
//

#include <algorithm>
#include <Utils/MemorySizes.h>
#include <RTTI/TypeInfo.h>
#include <RTTI/IntegralLayer.h>
#include <RTTI/EngineLayer.h>
#include "Core/RTTI/RTTIManager.h"

#include "Core/EngineApis.h"
#include "Core/MemoryModule.h"
#include "Core/WindowManager.h"
#include "Core/Input/InputManager.h"
#include "Core/Resources/ResourceManager.h"
#include "Core/Resources/Mesh/MeshManager.h"
#include "Core/Resources/Font/FontManager.h"
#include "Core/Resources/Shader/ShaderManager.h"
#include "Core/Resources/Texture/TextureManager.h"
#include "Core/Resources/Material/MaterialManager.h"
#include "Core/Gfx/BufferManager.h"
#include "Core/Gfx/Renderer.h"
#include "Core/Logic/Scene.h"
#include "Core/EventQueue.h"
#include "Core/GameTime.h"
#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Core/Config.h"

#include "Core/ManagerFactory.h"
#include "Core/DefaultDeleter.h"

#ifdef CreateWindow
#undef CreateWindow
#endif

namespace Core
{
    Engine::Engine(const char* name, std::size_t memorySize)
        : Name(name), _isDone(false), _cleanedUp(false), managers(Memory::GetDefaultBlock<Engine>())
        
		// Raw objects
		, MemoryModule(yage::DefaultDeleter<Core::MemoryModule>)
        , Config(yage::DefaultDeleter<Core::Config>)

		// Core::IManager
        , Renderer(yage::GetManagerDeleter<Gfx::Renderer>(this))
        , BufferManager(yage::GetManagerDeleter<Gfx::BufferManager>(this))
        , MeshManager(yage::GetManagerDeleter<Resources::MeshManager>(this))
        , FontManager(yage::GetManagerDeleter<Resources::FontManager>(this))
        , TextureManager(yage::GetManagerDeleter<Resources::TextureManager>(this))
        , MaterialManager(yage::GetManagerDeleter<Resources::MaterialManager>(this))
        , ShaderManager(yage::GetManagerDeleter<Resources::ShaderManager>(this))
        , WindowManager(yage::GetManagerDeleter<Core::WindowManager>(this))
        , InputManager(yage::GetManagerDeleter<Core::InputManager>(this))
        , RTTIManager(yage::GetManagerDeleter<RTTI::Manager>(this))
        
		// RTTI::ILayer
		, RTTIIntegralLayer(RTTI::Manager::DeleteLayer<RTTI::IntegralLayer>)
        , RTTIEngineLayer(RTTI::Manager::DeleteLayer<RTTI::EngineLayer>)
    {
        RTTI::SetupRTTI();

        MemoryModule	.reset(new Core::MemoryModule(memorySize));
        Config			.reset(new Core::Config(
            MemoryModule->requestMemoryBlock(Memory::KB(1), "Config Block"))
        );

        yage::CreateManager<Gfx::Renderer>             (*this, Renderer,        Memory::KB(100));
        yage::CreateManager<Gfx::BufferManager>        (*this, BufferManager,   Memory::KB(100));
        yage::CreateManager<Resources::MeshManager>    (*this, MeshManager,     Memory::KB(100));
        yage::CreateManager<Resources::FontManager>    (*this, FontManager,     Memory::KB(400));
        yage::CreateManager<Resources::TextureManager> (*this, TextureManager,  Memory::MB(4));
        yage::CreateManager<Resources::MaterialManager>(*this, MaterialManager, Memory::KB(100));
        yage::CreateManager<Resources::ShaderManager>  (*this, ShaderManager,   Memory::KB(100));
        yage::CreateManager<Core::WindowManager>       (*this, WindowManager,   Memory::KB(10));
        yage::CreateManager<Core::InputManager>        (*this, InputManager,    Memory::KB(10));
        yage::CreateManager<RTTI::Manager>             (*this, RTTIManager,     Memory::KB(10));

        // Integral types
        RTTIManager->PushLayer<RTTI::IntegralLayer>();

        // Engine CTTI
        RTTIManager->PushLayer<RTTI::EngineLayer>();

        glfwSetErrorCallback(&Engine::ErrorCallback);
    }

    Engine::~Engine()
    {
        CleanUp();
    }

    void Engine::ErrorCallback(int code, const char* description)
    {
        Logger::critical("GLFW : Error '{}' occured :\n\t{}", code, description);
    }

    void Engine::RegisterManager(borrowed_ptr<IManager>&& manager)
    {
        managers.add(std::move(manager));
    }

	void Engine::UnregisterManager(detail::owned_ptr_base& owning_ptr)
	{
		auto* itr = std::find_if(managers.begin(), managers.end(), [&](borrowed_ptr<IManager>& ptr) {
			return ptr.getBaseOwner() == &owning_ptr;
		});

		if (itr != managers.end())
		{
			managers.eraseAddress(itr);
		}
		else
		{
			YAGE_ASSERT(false, "Attempt to erase unregistered manager!");
		}
	}

    auto Engine::CreateWindow() const noexcept -> Window::handle_t
    {
        auto handle = WindowManager->createNew (
            ((std::string)Config->WindowTitle).c_str(),
             Config->WindowWidth,
             Config->WindowHeight
        );

        auto* window = WindowManager->tryGet(handle);
        if (window != nullptr)
        {
            if (!Renderer->registerWindow(window))
            {
                Logger::error("Engine : Unable to register window in OpenGL!");
            }

            if (!EventQueue::registerWindow(window))
            {
                Logger::error("Engine : Unable to register window in EventQueue!");
            }
        }
        else
        {
            Logger::error("Unable to create window!");
        }

        return handle;
    }

    auto Engine::LoadConfig(const char* path) -> bool
    {
        return Config->Load(path);
    }

    auto Engine::Initialize() -> bool
    {
        bool result = true;
        result &= OpenGL::initialize();
        result &= EventQueue::initialize(MemoryModule->requestMemoryBlock(Memory::KB(200), "EventQueueBlock"));
        return result;
    }

    auto Engine::SwitchScene(borrowed_ptr<Logic::Scene> scene) -> void
    {
        // todo: refactor to handle instead of raw pointer
        if (activeScene) activeScene->End();

        //activeScene = scene;

        if (activeScene) activeScene->Start();
    }

    // todo: remove window from here
    auto Engine::Draw(const Core::Window& /*window*/, Core::GameTime& time) -> void
    {
        if (WindowManager->allWindowsClosed())
            return;

        if (activeScene) activeScene->Draw(time, *Renderer.get());

        Renderer->draw();
        Renderer->drawSpriteBatches();
    }

    auto Engine::ProcessEvents(Core::GameTime& time) -> void
    {
        if (ShouldClose())
            return;

        Event event;
        while (EventQueue::pollEvent(&event))
        {
            switch (event.type)
            {
                case EventType::WINDOW:
                    WindowManager->handleWindowEvent(event);
                    break;
                case EventType::INPUT:
                    InputManager->handleInputEvent(event, time);
                    break;
                case EventType::APP:
                    break;
            }
        }
    }

    void Engine::Update(GameTime& time)
    {
        if (activeScene)
            activeScene->Update(time);

        if (WindowManager->allWindowsClosed())
            Quit();
    }

    double Engine::GetCurrentTime()
    {
        return glfwGetTime();
    }

    void Engine::Quit()
    {
        _isDone = true;
    }

    void Engine::ReleaseManagers()
    {
        // From last to first
        for (auto i = (std::int32_t) managers.size() - 1; i >= 0; i--)
        {
            Utils::borrowed_ptr<IManager>& ptr = managers[i];
            auto* owner = ptr.getBaseOwner();

            ptr.~borrowed_ptr();

            if (owner->hasBorrowers())
            {
                Logger::error("Manager at '{}' has unreleased borrowed_ptr's!", i);
            }

            owner->destroy();
        }

        managers.clear();
    }

    void Engine::CleanUp()
    {
        if (_cleanedUp)
            return;

        Logger::info("Cleaning up...");

        EventQueue::destroy();

        ReleaseManagers();

        glfwTerminate();

        Logger::info("Cleaned up!");

        Config.destroy();
        MemoryModule.destroy();

        RTTI::ShutdownRTTI();

        _cleanedUp = true;
    }

    bool Engine::WasCleanedUp()
    {
        return _cleanedUp;
    }

    bool Engine::ShouldClose()
    {
        return _isDone || WasCleanedUp();
    }
}
