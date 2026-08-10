import createEmscriptenModule from "./dist/domino-c.mjs";

export const wasmUrl = new URL("./dist/domino-c.wasm", import.meta.url).href;

export function createModule(options = {}) {
  return createEmscriptenModule({
    ...options,
    locateFile(path, prefix) {
      if (path === "domino-c.wasm") return wasmUrl;
      return options.locateFile?.(path, prefix) ?? `${prefix}${path}`;
    },
  });
}

export default createModule;
