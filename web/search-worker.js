import createF2LEngine from "./f2l_web.js";

const enginePromise = createF2LEngine();

self.onmessage = async ({ data }) => {
  const { operation, payload } = data;
  try {
    const engine = await enginePromise;
    self.postMessage({ type: "started" });
    let result;
    if (operation === "inspect") {
      result = engine.inspectCube(payload.moves);
    } else if (operation === "scramble") {
      result = engine.randomScramble(payload.count, payload.seed);
    } else if (operation === "search") {
      result = engine.searchCube(
        payload.mode, payload.setup, payload.target, payload.slot,
        payload.depth, payload.timeout, payload.beam, payload.pairPruning
      );
    } else if (operation === "reconstruct") {
      result = engine.reconstructF2L(
        payload.scramble, payload.cross, payload.lastLayer,
        payload.depth, payload.timeout, payload.beam
      );
    } else {
      throw new Error("Unknown browser operation");
    }
    self.postMessage({ type: "done", result });
  } catch (error) {
    self.postMessage({
      type: "error",
      message: error instanceof Error ? error.message : String(error)
    });
  }
};
