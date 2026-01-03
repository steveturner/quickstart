# CMake build type: Debug, Release, RelWithDebInfo
BUILD_TYPE ?= Debug

# Ditto SDK version and platform detection
DITTO_SDK_VERSION ?= 4.13.1
PLATFORM := $(shell uname -s | tr '[:upper:]' '[:lower:]')
ARCH := $(shell uname -m)

# Determine Ditto SDK platform string
ifeq ($(PLATFORM),linux)
    DITTO_PLATFORM = cpp-linux-$(ARCH)
    SDK_SUPPORTED = true
else
    # macOS and Windows are not supported by Ditto C++ SDK
    SDK_SUPPORTED = false
endif

DITTO_SDK_URL = https://software.ditto.live/$(DITTO_PLATFORM)/Ditto/$(DITTO_SDK_VERSION)/dist/Ditto.tar.gz

# Utility paths
CMAKE ?= cmake
CTEST ?= ctest

BUILD_DIR = build

# The "help" target will display all targets marked with a "##" comment.
.PHONY: help
help: MAKEFILE_FMT = "  \033[36m%-30s\033[0m%s\n"
help: ## (default) Displays this message
	@echo "Counter-UAS Sensor Mesh - Ditto C++ Quickstart"
	@echo ""
	@echo "Targets:"
	@grep -E '^[a-zA-Z0-9_-]*:.*?##' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?##"}; {printf $(MAKEFILE_FMT), $$1, $$2}'
	@echo ""
	@echo "Parameters:"
	@grep -E '^[A-Z0-9_-]* ?\?=.*?##' $(MAKEFILE_LIST) | awk 'BEGIN {FS = " ?\\?=.*?##"}; {printf $(MAKEFILE_FMT), $$1, $$2}'
	@echo ""
	@echo "Platform: $(PLATFORM) ($(ARCH))"
ifeq ($(SDK_SUPPORTED),true)
	@echo "Ditto SDK: Supported - will download v$(DITTO_SDK_VERSION)"
else
	@echo "Ditto SDK: Not supported on $(PLATFORM) - using stub for demo"
	@echo "           Run in Linux (devcontainer) for real P2P sync"
endif

.PHONY: download-sdk
download-sdk: ## Downloads and installs the Ditto C++ SDK (Linux only)
ifeq ($(SDK_SUPPORTED),true)
	@echo "📥 Downloading Ditto C++ SDK v$(DITTO_SDK_VERSION) for $(DITTO_PLATFORM)..."
	@if [ ! -f sdk/Ditto.h ] || [ ! -f sdk/libditto.a ]; then \
		mkdir -p sdk; \
		if curl -L -f $(DITTO_SDK_URL) | tar xz --strip-components=0 -C sdk/; then \
			echo "✅ Ditto C++ SDK v$(DITTO_SDK_VERSION) installed successfully"; \
		else \
			echo "❌ Failed to download SDK for $(DITTO_PLATFORM)"; \
			exit 1; \
		fi; \
	else \
		echo "✅ Ditto C++ SDK already installed"; \
	fi
else
	@echo "⚠️  Ditto C++ SDK only supports Linux x64/ARM64 platforms"
	@echo "   Current platform: $(PLATFORM) ($(ARCH))"
	@echo "   Using stub implementation for demo purposes"
	@echo ""
	@echo "   For real P2P sync, run in the devcontainer:"
	@echo "   - Open in VS Code with Remote Containers extension"
	@echo "   - Or run: docker-compose -f ../.devcontainer/docker-compose.yml up"
endif

.PHONY: build
build: download-sdk ## Build all targets (downloads SDK first on Linux)
ifeq ($(SDK_SUPPORTED),true)
	$(CMAKE) -B $(BUILD_DIR) . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DDITTO_SDK_PATH=$(CURDIR)/sdk -DBUILD_TESTS=OFF
else
	$(CMAKE) -B $(BUILD_DIR) . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DBUILD_TESTS=OFF
endif
	$(CMAKE) --build $(BUILD_DIR) --parallel

.PHONY: build-tests
build-tests: download-sdk ## Build with tests enabled
ifeq ($(SDK_SUPPORTED),true)
	$(CMAKE) -B $(BUILD_DIR) . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DDITTO_SDK_PATH=$(CURDIR)/sdk -DBUILD_TESTS=ON
else
	$(CMAKE) -B $(BUILD_DIR) . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DBUILD_TESTS=ON
endif
	$(CMAKE) --build $(BUILD_DIR) --parallel

.PHONY: run-demo
run-demo: build ## Build and run the single-process demo
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "  Starting Counter-UAS Sensor Mesh Demo"
ifeq ($(SDK_SUPPORTED),true)
	@echo "  Using Ditto SDK v$(DITTO_SDK_VERSION) - Real P2P Sync Enabled"
else
	@echo "  Using Stub - Demo mode only (no real P2P sync)"
endif
	@echo "═══════════════════════════════════════════════════════════"
	@echo ""
	cd $(BUILD_DIR) && ./cuas-demo

.PHONY: run-sensor
run-sensor: build ## Run as sensor node
	cd $(BUILD_DIR) && ./cuas-mesh --mode sensor --id RADAR-1

.PHONY: run-c2
run-c2: build ## Run as C2 node
	cd $(BUILD_DIR) && ./cuas-mesh --mode c2 --id C2-ALPHA

.PHONY: run-effector
run-effector: build ## Run as effector node
	cd $(BUILD_DIR) && ./cuas-mesh --mode effector --id EFFECTOR-1

.PHONY: run-all
run-all: build ## Run all nodes in separate tmux panes
	@if ! command -v tmux &> /dev/null; then \
		echo "❌ tmux is required. Install with: brew install tmux (macOS) or apt install tmux (Linux)"; \
		exit 1; \
	fi
	@echo "Starting all nodes in tmux..."
	tmux new-session -d -s cuas-mesh "cd $(BUILD_DIR) && ./cuas-mesh --mode sensor --id RADAR-1"
	tmux split-window -h "cd $(BUILD_DIR) && ./cuas-mesh --mode c2 --id C2-ALPHA"
	tmux split-window -v "cd $(BUILD_DIR) && ./cuas-mesh --mode effector --id EFFECTOR-1"
	tmux select-layout tiled
	tmux attach-session -t cuas-mesh

.PHONY: test
test: build-tests ## Run all tests
	cd $(BUILD_DIR) && $(CTEST) --output-on-failure

.PHONY: clean
clean: ## Remove build artifacts
	rm -rf $(BUILD_DIR)
	rm -f src/env.h

.PHONY: clean-sdk
clean-sdk: ## Remove downloaded SDK
	rm -rf sdk/Ditto.h sdk/libditto.a sdk/*.h sdk/*.a

.PHONY: clean-all
clean-all: clean clean-sdk ## Remove all generated files

.PHONY: info
info: ## Display build information
	@echo "Platform:     $(PLATFORM) ($(ARCH))"
	@echo "SDK Version:  $(DITTO_SDK_VERSION)"
	@echo "SDK Platform: $(DITTO_PLATFORM)"
	@echo "SDK URL:      $(DITTO_SDK_URL)"
	@echo "Build Type:   $(BUILD_TYPE)"
ifeq ($(SDK_SUPPORTED),true)
	@echo "SDK Status:   ✅ Supported"
else
	@echo "SDK Status:   ⚠️  Not supported (using stub)"
endif
