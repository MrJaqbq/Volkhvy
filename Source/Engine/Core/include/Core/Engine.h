//
// Created by mrjaqbq on 07.03.16.
//

#ifndef YAGE_ENGINE_H
#define YAGE_ENGINE_H

#include <memory>
#include <string>
#include <unordered_map>

#include <Utils/Defines.h>
#include <Utils/Handle.h>
#include <Utils/List.h>
#include <Utils/OwnedPtr.h>

#include "MemoryModule.h"

namespace RTTI
{
	class Manager;
	class IntegralLayer;
	class EngineLayer;
}

namespace Logic
{
	class Scene;

	class SceneManager;
}

namespace Gfx
{
	class Renderer;
	class BufferManager;
}

namespace Resources
{
	class MeshManager;
	class FontManager;
	class TextureManager;
	class MaterialManager;
	class ShaderManager;
}

namespace Scripts
{
	class ScriptManager;
}

namespace Core
{
	using namespace Utils;

	class Config;

	class Logger;

	class Window;

	class WindowManager;

	class MessageDispather;

	class InputManager;

	struct Context;

	struct GameTime;

	class IManager;

	class Engine
	{
		borrowed_ptr<Logic::Scene> activeScene;

		List<borrowed_ptr<IManager>> managers;

		owned_ptr<RTTI::IntegralLayer> RTTIIntegralLayer;
		owned_ptr<RTTI::EngineLayer>   RTTIEngineLayer;

		bool _cleanedUp = false;
		bool _isDone = false;

		static void ErrorCallback(int code, const char* description);

		void ReleaseManagers();

	public:

		static void initializeReferences(Engine* engine);

		Utils::String Name;

		owned_ptr<Core::MemoryModule>			MemoryModule;
		owned_ptr<Core::Config>					Config;
		owned_ptr<Gfx::Renderer>				Renderer;
		owned_ptr<Gfx::BufferManager>			BufferManager;
		owned_ptr<Resources::MeshManager>		MeshManager;
		owned_ptr<Resources::FontManager>		FontManager;
		owned_ptr<Resources::TextureManager>	TextureManager;
		owned_ptr<Resources::MaterialManager>	MaterialManager;
		owned_ptr<Resources::ShaderManager>		ShaderManager;
		owned_ptr<Core::WindowManager>			WindowManager;
		owned_ptr<Core::InputManager>			InputManager;
		owned_ptr<RTTI::Manager>			    RTTIManager;

		explicit Engine(const char* name, std::size_t memorySize);

		virtual ~Engine();

		Engine(Engine&&) = delete;
		Engine& operator=(Engine&&) = delete;

		// Create Window based on current configuration
		auto CreateWindow() const noexcept -> Utils::Handle<Window>;

		// Load configuration
		bool LoadConfig(const char* = "Config.json");

		// Initialize graphics context based on current config
		bool Initialize();

		// Change active scene
		void SwitchScene(borrowed_ptr<Logic::Scene> scene);

		// Draw active scene
		void Draw(const Core::Window& window, Core::GameTime& time);

		// Update active scene
		void Update(Core::GameTime& time);

		// Return current time in ms
		auto GetCurrentTime() -> double;

		// Process user input
		void ProcessEvents(Core::GameTime& time);

		// Free all resources
		void CleanUp();

		// Call this when user wants to quit
		void Quit();

		bool WasCleanedUp();

		bool ShouldClose();

		void RegisterManager(borrowed_ptr<IManager>&& manager);
		
		void Engine::UnregisterManager(detail::owned_ptr_base& owning_ptr);

		//template <typename T>
		//void LoadRTTI(owned_ptr<T>& ptr)
		//{
		//	static_assert(std::is_base_of<RTTI::ILayer, T>::value,
		//		"Engine: Cannot register RTTI later that doesn't derive from ILayer!");

		//	T* instance = YAGE_CREATE_NEW(Memory::GetDefaultBlock<Engine>(), T)();
		//	ptr.reset(instance);
		//	RTTIManager.LoadLayer(ptr.borrow());
		//}
	};
}

#endif //YAGE_ENGINE_H
