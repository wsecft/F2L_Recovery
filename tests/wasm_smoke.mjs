import createF2LEngine from "../build-wasm/web/f2l_web.js";

const engine = await createF2LEngine();
const solved = engine.inspectCube("");
if (!solved.f2lSolved || !solved.crossSolved) {
  throw new Error("WASM cube inspection did not recognize the solved state");
}

const search = engine.searchCube("f2l", "", "", 0, 4, 5000, 500, false);
if (!search.found || !search.verified || search.macroCount !== 0) {
  throw new Error("WASM F2L search smoke test failed");
}

const reconstruction = engine.reconstructF2L(
  "x2 R U x2", "U' R'", "", 0, 1000, 500
);
if (!reconstruction.found || reconstruction.inspectionMoves !== "x2" ||
    reconstruction.crossFace !== "U" || reconstruction.moves !== "") {
  throw new Error("WASM inspection-frame reconstruction smoke test failed");
}

const scramble = engine.randomScramble(20, 42);
if (engine.inspectCube(scramble).facelets.length !== 54) {
  throw new Error("WASM scramble generation smoke test failed");
}

console.log("WASM engine smoke tests passed.");
