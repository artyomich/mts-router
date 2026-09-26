#!/usr/bin/env python3
"""MTS Router — Test runner. Runs all device API tests."""
import json
import os
import subprocess
import sys
from pathlib import Path

RED = '\033[0;31m'
GREEN = '\033[0;32m'
YELLOW = '\033[1;33m'
NC = '\033[0m'

SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_DIR = SCRIPT_DIR.parent
BUILD_DIR = PROJECT_DIR / 'build'
TEST_DIR = BUILD_DIR / 'test-results'
LOG_DIR = BUILD_DIR / 'logs'

TEST_DIR.mkdir(parents=True, exist_ok=True)
LOG_DIR.mkdir(parents=True, exist_ok=True)

results = []
failures = []


def log(color, msg):
    print(f"{color}{msg}{NC}")


def run_test(name, func):
    try:
        func()
        log(GREEN, f"[PASS] {name}")
        results.append((name, True))
    except Exception as e:
        log(RED, f"[FAIL] {name}")
        failures.append((name, str(e)))
        results.append((name, False))


def test_proto_syntax_core():
    content = (PROJECT_DIR / 'core-router-api/proto/mts_core_router.proto').read_text()
    assert 'syntax = "proto3"' in content
    assert 'package mts.core.router.v1' in content
    assert 'service MtsCoreRouterService' in content
    assert 'rpc GetFabricStatus' in content


def test_proto_syntax_mobile_core():
    content = (PROJECT_DIR / 'mobile-core-api/proto/mts_mobile_core.proto').read_text()
    assert 'syntax = "proto3"' in content
    assert 'package mts.mobile.core.v1' in content
    assert 'service MtsMobileCoreService' in content
    assert 'rpc GetUpfStatus' in content


def test_proto_syntax_mobile_backhaul():
    content = (PROJECT_DIR / 'mts-mb3000-api/proto/mts_backhaul.proto').read_text()
    assert 'syntax = "proto3"' in content
    assert 'package mts.backhaul.v1' in content
    assert 'service MtsBackhaulService' in content
    assert 'rpc GetPtpStatus' in content


def test_proto_syntax_olt_gpon():
    content = (PROJECT_DIR / 'olt-gpon-api/proto/mts_olt_gpon.proto').read_text()
    assert 'syntax = "proto3"' in content
    assert 'package mts.olt.gpon.v1' in content
    assert 'service MtsOltGponService' in content
    assert 'rpc GetOltStatus' in content


def test_proto_syntax_enterprise():
    content = (PROJECT_DIR / 'enterprise-router-api/proto/mts_enterprise.proto').read_text()
    assert 'syntax = "proto3"' in content
    assert 'package mts.enterprise.v1' in content
    assert 'service MtsEnterpriseService' in content
    assert 'rpc GetSdwanStatus' in content


def test_proto_syntax_residential():
    content = (PROJECT_DIR / 'residential-gateway-api/proto/mts_residential.proto').read_text()
    assert 'syntax = "proto3"' in content
    assert 'package mts.residential.v1' in content
    assert 'service MtsResidentialService' in content
    assert 'rpc GetGponStatus' in content


def test_python_client_import():
    # Verify the examples file exists and has content (it's markdown with code blocks)
    p = PROJECT_DIR / 'api/examples/mts-api-examples.py'
    assert p.is_file()
    text = p.read_text()
    assert 'class MtsRouterClient' in text
    assert 'class MtsCoreRouterClient' in text
    assert 'class MtsMobileCoreClient' in text
    assert 'class MtsOltGponClient' in text
    assert 'class MtsResidentialGatewayClient' in text


def test_dir_structure_core():
    d = PROJECT_DIR / 'core-router-api'
    assert d.is_dir()
    assert (d / 'CMakeLists.txt').is_file()
    assert (d / 'proto/mts_core_router.proto').is_file()
    assert (d / 'config/mts-cr9000.conf').is_file()


def test_dir_structure_mobile_core():
    d = PROJECT_DIR / 'mobile-core-api'
    assert d.is_dir()
    assert (d / 'CMakeLists.txt').is_file()
    assert (d / 'proto/mts_mobile_core.proto').is_file()
    assert (d / 'config/mts-mc5000.conf').is_file()


def test_dir_structure_olt_gpon():
    d = PROJECT_DIR / 'olt-gpon-api'
    assert d.is_dir()
    assert (d / 'CMakeLists.txt').is_file()
    assert (d / 'proto/mts_olt_gpon.proto').is_file()
    assert (d / 'config/mts-olt2000.conf').is_file()


def test_dir_structure_enterprise():
    d = PROJECT_DIR / 'enterprise-router-api'
    assert d.is_dir()
    assert (d / 'CMakeLists.txt').is_file()
    assert (d / 'proto/mts_enterprise.proto').is_file()
    assert (d / 'config/mts-er1000.conf').is_file()


def test_dir_structure_residential():
    d = PROJECT_DIR / 'residential-gateway-api'
    assert d.is_dir()
    assert (d / 'CMakeLists.txt').is_file()
    assert (d / 'proto/mts_residential.proto').is_file()
    assert (d / 'config/mts-rg500.conf').is_file()


def test_scripts_executable():
    for name in ['build-all.sh', 'build-core-router.sh', 'build-mobile-core.sh',
                  'build-mobile-backhaul.sh', 'build-olt-gpon.sh', 'build-enterprise.sh',
                  'build-residential.sh']:
        p = SCRIPT_DIR / name
        assert p.exists(), f"{name} not found"
        assert os.access(p, os.X_OK), f"{name} not executable"


def test_specs_exist():
    for name in ['core-router/spec/core-router-spec.md',
                 'mobile-core/spec/mobile-core-spec.md',
                 'mobile-backhaul/spec/mobile-backhaul-spec.md',
                 'olt-gpon/spec/olt-gpon-spec.md',
                 'enterprise-router/spec/enterprise-router-spec.md',
                 'residential-gateway/spec/residential-gateway-spec.md']:
        p = PROJECT_DIR / name
        assert p.is_file(), f"{name} not found"


def test_api_spec_exists():
    assert (PROJECT_DIR / 'api/spec/mts-router-api.md').is_file()
    assert (PROJECT_DIR / 'api/spec/mts-api-protobuf.md').is_file()
    assert (PROJECT_DIR / 'api/examples/mts-api-examples.py').is_file()


def test_docs_exist():
    for name in ['docs/architecture-overview.md', 'docs/chipset-analysis.md',
                 'docs/linux-os-selection.md']:
        p = PROJECT_DIR / name
        assert p.is_file(), f"{name} not found"


def test_cmake_syntax_core():
    content = (PROJECT_DIR / 'core-router-api/CMakeLists.txt').read_text()
    assert 'cmake_minimum_required' in content
    assert 'find_package(protobuf' in content
    assert 'find_package(gRPC' in content


def test_cmake_syntax_mobile_core():
    content = (PROJECT_DIR / 'mobile-core-api/CMakeLists.txt').read_text()
    assert 'cmake_minimum_required' in content
    assert 'find_package(protobuf' in content
    assert 'find_package(gRPC' in content


def test_cmake_syntax_olt_gpon():
    content = (PROJECT_DIR / 'olt-gpon-api/CMakeLists.txt').read_text()
    assert 'cmake_minimum_required' in content
    assert 'find_package(protobuf' in content
    assert 'find_package(gRPC' in content


def test_cmake_syntax_enterprise():
    content = (PROJECT_DIR / 'enterprise-router-api/CMakeLists.txt').read_text()
    assert 'cmake_minimum_required' in content
    assert 'find_package(protobuf' in content
    assert 'find_package(gRPC' in content


def test_cmake_syntax_residential():
    content = (PROJECT_DIR / 'residential-gateway-api/CMakeLists.txt').read_text()
    assert 'cmake_minimum_required' in content
    assert 'find_package(protobuf' in content
    assert 'find_package(gRPC' in content


def test_config_json_core():
    data = json.loads((PROJECT_DIR / 'core-router-api/config/mts-cr9000.conf').read_text())
    assert 'device_id' in data
    assert data['device_id'] == 'MTS-CR-9000-001'


def test_config_json_mobile_core():
    data = json.loads((PROJECT_DIR / 'mobile-core-api/config/mts-mc5000.conf').read_text())
    assert 'device_id' in data
    assert data['device_id'] == 'MTS-MC-5000-001'


def test_config_json_olt_gpon():
    data = json.loads((PROJECT_DIR / 'olt-gpon-api/config/mts-olt2000.conf').read_text())
    assert 'device_id' in data
    assert data['device_id'] == 'MTS-OLT-2000-001'


def test_config_json_enterprise():
    data = json.loads((PROJECT_DIR / 'enterprise-router-api/config/mts-er1000.conf').read_text())
    assert 'device_id' in data
    assert data['device_id'] == 'MTS-ER-1000-001'


def test_config_json_residential():
    data = json.loads((PROJECT_DIR / 'residential-gateway-api/config/mts-rg500.conf').read_text())
    assert 'device_id' in data
    assert data['device_id'] == 'MTS-RG-500-001'


def test_agents_md_exists():
    assert (PROJECT_DIR / 'AGENTS.md').is_file()


def test_readme_exists():
    assert (PROJECT_DIR / 'README.md').is_file()


def test_project_summary_exists():
    assert (PROJECT_DIR / 'PROJECT-SUMMARY.md').is_file()


def test_mb3000_intact():
    for name in ['CMakeLists.txt', 'README.md', 'src/main.cpp', 'proto/mts_backhaul.proto']:
        p = PROJECT_DIR / 'mts-mb3000-api' / name
        assert p.is_file(), f"{name} not found in mts-mb3000-api"


if __name__ == '__main__':
    print("=== MTS Router — Test Suite ===")
    print(f"Start: {subprocess.check_output(['date', '-u', '+%Y-%m-%dT%H:%M:%SZ']).decode().strip()}")
    print()

    # Run all tests
    run_test("proto-syntax-core", test_proto_syntax_core)
    run_test("proto-syntax-mobile-core", test_proto_syntax_mobile_core)
    run_test("proto-syntax-mobile-backhaul", test_proto_syntax_mobile_backhaul)
    run_test("proto-syntax-olt-gpon", test_proto_syntax_olt_gpon)
    run_test("proto-syntax-enterprise", test_proto_syntax_enterprise)
    run_test("proto-syntax-residential", test_proto_syntax_residential)
    run_test("python-client-import", test_python_client_import)
    run_test("dir-structure-core", test_dir_structure_core)
    run_test("dir-structure-mobile-core", test_dir_structure_mobile_core)
    run_test("dir-structure-olt-gpon", test_dir_structure_olt_gpon)
    run_test("dir-structure-enterprise", test_dir_structure_enterprise)
    run_test("dir-structure-residential", test_dir_structure_residential)
    run_test("scripts-executable", test_scripts_executable)
    run_test("specs-exist", test_specs_exist)
    run_test("api-spec-exists", test_api_spec_exists)
    run_test("docs-exist", test_docs_exist)
    run_test("cmake-syntax-core", test_cmake_syntax_core)
    run_test("cmake-syntax-mobile-core", test_cmake_syntax_mobile_core)
    run_test("cmake-syntax-olt-gpon", test_cmake_syntax_olt_gpon)
    run_test("cmake-syntax-enterprise", test_cmake_syntax_enterprise)
    run_test("cmake-syntax-residential", test_cmake_syntax_residential)
    run_test("config-json-core", test_config_json_core)
    run_test("config-json-mobile-core", test_config_json_mobile_core)
    run_test("config-json-olt-gpon", test_config_json_olt_gpon)
    run_test("config-json-enterprise", test_config_json_enterprise)
    run_test("config-json-residential", test_config_json_residential)
    run_test("agents-md-exists", test_agents_md_exists)
    run_test("readme-exists", test_readme_exists)
    run_test("project-summary-exists", test_project_summary_exists)
    run_test("mb3000-intact", test_mb3000_intact)

    total = len(results)
    passed = sum(1 for _, ok in results if ok)
    failed = total - passed

    print()
    print("=== Test Results ===")
    print(f"Total: {total}, Passed: {passed}, Failed: {failed}")

    if failures:
        print()
        for name, err in failures:
            print(f"  {RED}[FAIL]{NC} {name}: {err}")
        log(RED, f"\nSome tests failed. Check logs in {TEST_DIR}/")
        sys.exit(1)

    log(GREEN, "\nAll tests passed!")
    sys.exit(0)
