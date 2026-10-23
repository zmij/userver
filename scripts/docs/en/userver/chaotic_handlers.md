## Chaotic handlers (server side)

Chaotic can generate C++ server-side handler base classes, request/response types, and
view stub files directly from an OpenAPI 3.x or Swagger 2.0 schema.
This eliminates hand-written boilerplate and keeps the server contract in sync with the schema.

For generating HTTP clients instead, see @ref scripts/docs/en/userver/chaotic_clients.md.


### What gets generated

For each HTTP operation in the schema chaotic produces:

* **Handler base class** — a userver `HttpHandlerBase` subclass with typed `Handle()` method.
* **Request / response types** — C++ structs with JSON parsers and serializers. A media type without a schema,
  e.g. `application/json: {}`, carries any value: `formats::json::Value` for JSON.
* **View stub** — a minimal `.hpp`/`.cpp` pair in `SRC_DIR` that you fill in with business logic.
  Existing view stubs are **never overwritten** on subsequent runs.
* **`config.chaotic.yaml`** — static config fragment for all generated handlers; merged into
  the service config by `userver_generate_config_yaml()`.


### Quickstart

**Step 1.** Write an OpenAPI schema (OpenAPI 3.x YAML or Swagger 2.0 JSON/YAML):

@include samples/chaotic_openapi_service/handlers/insecure/openapi.yaml

**Step 2.** Declare the handler target in `CMakeLists.txt`:

@snippet samples/chaotic_openapi_service/CMakeLists.txt chaotic-handler

**Step 3.** Implement the generated view stub. Chaotic writes a skeleton to
`src/handlers/NAME/OPERATION/view.cpp` on the first run:

@snippet samples/chaotic_openapi_service/src/handlers/insecure/insecuresecretpost/view.cpp view-impl

Subsequent runs leave `view.cpp` untouched.

**Step 4.** Register the handler list in `main.cpp`:

@snippet samples/chaotic_openapi_service/main.cpp register-handlers

**Step 5.** Merge the generated `config.chaotic.yaml` into the service config:

@snippet samples/chaotic_openapi_service/CMakeLists.txt generate-config


### CMake reference

#### `userver_target_generate_openapi_handlers(TARGET)`

| Parameter | Kind | Default | Description |
|---|---|---|---|
| First positional (`TARGET`) | — | **required** | CMake library target name to create. |
| `NAME` | one-value | **required** | Service name; used in generated include paths (`handlers/NAME/…`) and as the default C++ namespace base. |
| `SCHEMAS` | multi-value | **required** | OpenAPI YAML/JSON source files. |
| `OUTPUT_DIR` | one-value | `${CMAKE_CURRENT_BINARY_DIR}/NAME`, `${CMAKE_CURRENT_BINARY_DIR}` for tagged schemas | Destination for generated `include/` and `src/` trees. |
| `SRC_DIR` | one-value | — | If set, view stubs for new operations are written here under `handlers/NAME/OP/`, or under the handlers' directory for tagged schemas. Omit to skip view-stub generation. |
| `ARGS` | multi-value | — | Extra arguments forwarded verbatim to `chaotic-openapi-gen`. |

The macro links the target against `userver::chaotic-openapi` and exposes the generated
`include/` directory. It also attaches the generated `config.chaotic.yaml` path to the target
via the `INTERFACE_USERVER_EXTRA_CONFIG_YAML_PATHS` property, which
`userver_generate_config_yaml()` collects transitively.

#### `userver_generate_config_yaml(BINARY_TARGET)`

Merges `config.chaotic.yaml` files collected from all handler targets in the link graph of
`BINARY_TARGET` with the caller-provided base configs, producing a single `config.yaml`.

| Parameter | Kind | Default | Description |
|---|---|---|---|
| First positional (`BINARY_TARGET`) | — | **required** | Executable or library whose transitive link graph is scanned for `config.chaotic.yaml` files. |
| `OUTPUT` | one-value | **required** | Path for the final merged `config.yaml`. |
| `BASE_CONFIGS` | multi-value | — | User-provided config fragments applied last (highest priority). |

The macro creates a `${BINARY_TARGET}_config` target that depends on the output file.


### Generated file layout

For a service named `greeter` with an operation `GET /hello` (operationId `getHello`), the
layout under `OUTPUT_DIR` is:

```
include/handlers/greeter/
    get_hello/
        handler.hpp      # handler base class
        requests.hpp     # request types
        responses.hpp    # response types
    openapi.hpp          # shared schema types
src/handlers/greeter/
    get_hello/
        handler.cpp
        requests.cpp
        responses.cpp
    openapi.cpp
config.chaotic.yaml      # static config fragment
```

View stubs (written to `SRC_DIR/handlers/greeter/get_hello/`):

```
view.hpp    # view interface declaration
view.cpp    # view stub — implement this
```


### Parameter and header types

Request parameters and response headers get the same C++ types as in generated clients,
see @ref chaotic_clients_parameter_types.


### Authentication

When the OpenAPI schema declares a `securityScheme` on a handler operation,
the generated `config.chaotic.yaml` includes the scheme name in `auth.types`, whatever the type of the scheme
(`http`, `apiKey`, `oauth2` or `openIdConnect`):

@snippet samples/chaotic_openapi_service/clients/secure.yaml security requirement

Generate the handler and merge its generated config with the service config:

@snippet samples/chaotic_openapi_service/CMakeLists.txt digest-handler-cmake

Register an auth checker factory under the same security-scheme name:

@snippet samples/chaotic_openapi_service/main.cpp digest-auth-checker-registration

@snippet samples/chaotic_openapi_service/src/auth_digest.hpp auth checker factory

The reusable standalone Digest factory obtains the nonce-cache settings from the component list:

@snippet samples/chaotic_openapi_service/main.cpp digest-settings-component

Options that code generation cannot know, such as the Digest realm, stay in the service config:

@snippet samples/chaotic_openapi_service/static_config.user.yaml digest-handler-auth-config

See @ref scripts/docs/en/userver/tutorial/digest_auth_postgres.md for a database-backed
Digest auth checker. The `chaotic_openapi_service` sample contains a complete in-memory example.

#### Scopes

A security requirement may name the scopes an operation needs. OpenAPI 3.0 allows them only for `oauth2` and
`openIdConnect` schemes; from OpenAPI 3.1 on any scheme may list role names. The scopes are passed to the auth
checker in `auth.scopes`, the checker's factory reads them from the `HandlerAuthConfig` it receives:

```yaml
# openapi.yaml
paths:
  /accounts/{id}:
    delete:
      security:
        - permissions: [accounts:manage]

# config.chaotic.yaml
handler-accounts-id-delete:
  auth:
    types: [permissions]
    scopes: [accounts:manage]
```

`oauth2` schemes declare their scopes in `flows`. Any other scheme may declare them in `x-usrv-scopes`, a map of
scope names to descriptions:

```yaml
components:
  securitySchemes:
    permissions:
      type: apiKey
      in: header
      name: X-Permissions
      x-usrv-scopes:
        accounts:manage: Manage accounts
```

When the scopes are declared, the generator rejects a security requirement that names a scope missing from them.


### Namespace and path tags

Schemas may declare their C++ namespace with `x-usrv-cpp-namespace` and, optionally, the path of their generated
files with `x-usrv-cpp-path`, see @ref chaotic_namespace_tags. Then the handlers live in the sub-namespace
`handlers` of the paths document's namespace, and their files under the `handlers` subdirectory of the document's
directory. A client generated from the same document gets the sub-namespace `client`, so the two never clash.

For a paths document tagged `greeter::api` with an operation `GET /hello` (operationId `getHello`), the layout under
`OUTPUT_DIR` (by default `${CMAKE_CURRENT_BINARY_DIR}`) is:

```
include/greeter/api/handlers/
    gethello/
        handler.hpp      # greeter::api::handlers::gethello
        requests.hpp
        responses.hpp
    chaotic_handlers_list.hpp
src/greeter/api/handlers/
    ...
greeter/api/handlers/config.chaotic.yaml
```

The handlers generate no types. The types of the document, its `components/schemas` and the inline schemas of its
operations, are generated once into `include/greeter/api/openapi.hpp`, in the namespace `greeter::api`, see
@ref chaotic_namespace_tags: the first of the handlers and client targets of the document adds a target generating
them, the other one links it.

View stubs are written to `SRC_DIR/greeter/api/handlers/gethello/`. The handler component is named after the
namespace and the operation, `greeter-api-handlers-get-hello`, so handlers of different documents never clash
either.

Schemas referred to from other files are not copied either: their headers are included, and the files must be
generated by `userver_target_generate_chaotic()` targets defined before the handlers target. In the `chaotic_tagged_service` sample the handler passes the
`Greeting` and the `Error` its generated client receives straight through, they are the same types:

@snippet samples/chaotic_tagged_service/src/samples/tagged/greeter/handlers/getgreeting/view.cpp view

`--namespace` is forbidden, and all `SCHEMAS` of a target are either tagged or not.

The CMake function asks the generator for the files to generate, view stubs included, and for the tagged schemas the
`SCHEMAS` refer to at configure time, the target regenerates when any of them changes. When calling
`chaotic-openapi-gen` directly, `--src-dir` is the root of the view sources, the stubs go to the handlers' directory
under it:

```
chaotic-openapi-gen --gen types -o build/gen api/openapi.yaml
chaotic-openapi-gen --gen handlers+views --name greeter -o build/gen --src-dir src api/openapi.yaml
chaotic-openapi-gen --gen handlers+views --name greeter -o build/gen --src-dir src --list-outputs api/openapi.yaml
chaotic-openapi-gen --gen handlers --name greeter -o build/gen --list-dependencies api/openapi.yaml
```


### Handler policy {#chaotic_handler_policy}

How a handler logs request and response bodies and renders the errors its operation does not declare may be set
per handler or for a whole list of handlers, at the point the handlers are registered. A policy is a type deriving
from `chaotic::openapi::server::DefaultHandlerPolicy` that redefines the hooks it needs; each hook returns
`std::nullopt` to keep the userver default:

@snippet samples/chaotic_openapi_service/src/handler_policy.hpp handler policy

| Hook | Used for |
|---|---|
| `GetRequestBodyForLogging(request, context, request_body)` | The request body written to the logs. |
| `GetResponseDataForLogging(request, context, response_data)` | The response body written to the logs. |
| `GetFormattedExternalErrorBody(exc)` | The body of an error the operation does not declare: a request that fails to parse, a `server::handlers::CustomHandlerException` or any other exception escaping the view. Declared error responses are serialized as declared. |

Every generated operation provides `HandlerFor<Policy>` beside `Handler`, which is `HandlerFor<>` without a policy,
and the generated `ChaoticHandlersList<Policy>()` applies a policy to all the handlers of the list:

@snippet samples/chaotic_openapi_service/main.cpp register-handlers

To use different policies, append the handlers one by one:

```cpp
component_list
    .Append<handlers::insecure::callsecretget::HandlerFor<LoudPolicy>>()
    .Append<handlers::insecure::insecuresecretpost::HandlerFor<RedactingPolicy>>();
```

A View's own `GetRequestBodyForLogging`, `GetInvalidRequestBodyForLogging` and `GetResponseForLogging` take precedence
over the policy, the policy over the userver default. Handlers registered without a policy behave exactly as before.

A policy hook with a wrong signature does not compile, and neither does a View hook whose signature matches none of
the supported ones: a mistyped hook is reported instead of being silently ignored.


### Direct CLI reference

`chaotic-openapi-gen` is the underlying executable. Handler-relevant flags:

| Flag | Required | Description |
|---|---|---|
| `files …` | yes | One or more OpenAPI/Swagger YAML/JSON files. |
| `--name` | unless `--gen types` | Service name (sets include-path prefix and default namespace). |
| `--gen` | yes | `handlers` — handler classes only; `handlers+views` — handlers and view stubs; `views` — view stubs only; `types` — the types of tagged documents, see @ref chaotic_namespace_tags. |
| `-o` / `--output-dir` | for `handlers`/`handlers+views` | Destination for generated `include/` and `src/`. |
| `--src-dir` | for `views`/`handlers+views` | Directory where view stubs are written; for tagged schemas, the root of the view sources. |
| `--namespace` | no | Override the C++ namespace (default: `handlers::NAME`). Forbidden for tagged schemas. |
| `--list-outputs` | no | Print the files to generate, view stubs included, `;`-separated, and exit. |
| `--list-dependencies` | no | Print the tagged schemas generated elsewhere that the inputs refer to, `;`-separated, and exit. |
| `-I` / `--include-dir` | no (repeatable) | Extra include path for `x-usrv-cpp-type` header lookup. |
| `--clang-format` | no | clang-format binary; set to empty string to disable. |
| `-u` / `--userver` | no | userver namespace macro name (default: `USERVER_NAMESPACE`). |


----------

@htmlonly <div class="bottom-nav"> @endhtmlonly
⇦ @ref scripts/docs/en/userver/chaotic_clients.md | @ref scripts/docs/en/userver/sql_files.md ⇨
@htmlonly </div> @endhtmlonly
