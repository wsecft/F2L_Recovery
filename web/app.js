const colors = ["u", "r", "f", "d", "l", "b"];
const faces = [
  ["U", "u"], ["L", "l"], ["F", "f"],
  ["R", "r"], ["B", "b"], ["D", "d"]
];
const preset = {
  scramble: "L2 U2 L D' F R F' R2 D2 L' B2 R' D2 R2 U2 L B2 R D B",
  cross: "U' L F' L2 R' B2",
  lastLayer: "F R' F' r U R U' r' U'"
};

const engineBadge = document.querySelector("#engine-state");
const cubeNet = document.querySelector("#cube-net");
const setupInput = document.querySelector("#setup-moves");
let activeWorker = null;
let activeCancel = null;
let currentSetup = "";

for (const [label, face] of faces) {
  const faceElement = document.createElement("div");
  faceElement.className = `face face-${face}`;
  faceElement.setAttribute("aria-label", `${label} face`);
  faceElement.dataset.face = label;
  for (let i = 0; i < 9; i++) {
    const sticker = document.createElement("span");
    sticker.className = "sticker";
    faceElement.append(sticker);
  }
  cubeNet.append(faceElement);
}

function setEngineState(text, status) {
  engineBadge.textContent = text;
  engineBadge.className = `engine-state ${status}`;
}

function makeWorkerRequest(operation, payload, onProgress = () => {}) {
  if (activeWorker) cancelWorker();
  return new Promise((resolve, reject) => {
    let worker;
    try {
      worker = new Worker(new URL("./search-worker.js", import.meta.url), {
        type: "module"
      });
    } catch (error) {
      reject(error);
      return;
    }
    activeWorker = worker;
    activeCancel = () => {
      activeWorker = null;
      activeCancel = null;
      worker.terminate();
      const error = new Error("Search cancelled");
      error.name = "AbortError";
      reject(error);
    };
    worker.onmessage = ({ data }) => {
      if (data.type === "started") {
        setEngineState("WebAssembly ready", "ready");
        onProgress();
      } else if (data.type === "done") {
        activeWorker = null;
        activeCancel = null;
        worker.terminate();
        resolve(data.result);
      } else if (data.type === "error") {
        activeWorker = null;
        activeCancel = null;
        worker.terminate();
        reject(new Error(data.message));
      }
    };
    worker.onerror = (error) => {
      activeWorker = null;
      activeCancel = null;
      worker.terminate();
      reject(new Error(error.message || "Unable to load the WebAssembly engine"));
    };
    worker.postMessage({ operation, payload });
  });
}

function cancelWorker() {
  if (activeCancel) activeCancel();
}

function displayFacelets(faceletString) {
  if (typeof faceletString !== "string" || faceletString.length !== 54) return;
  const allFaces = cubeNet.querySelectorAll(".face");
  allFaces.forEach((faceElement) => {
    const faceIndex = colors.indexOf(faceElement.dataset.face.toLowerCase());
    const stickers = faceElement.querySelectorAll(".sticker");
    stickers.forEach((sticker, position) => {
      const colorIndex = Number(faceletString[faceIndex * 9 + position]);
      sticker.className = `sticker sticker-${colors[colorIndex] ?? "u"}`;
    });
  });
}

function updateCubeInfo(info, description = "") {
  displayFacelets(info.facelets);
  const labels = [];
  if (info.crossSolved) labels.push("cross solved");
  if (info.f2lSolved) labels.push("F2L solved");
  const status = labels.length ? labels.join(" · ") : "cross and F2L incomplete";
  document.querySelector("#frame-badge").textContent =
    info.f2lSolved ? "F2L solved" : info.crossSolved ? "Cross solved" : "In progress";
  document.querySelector("#cube-status").textContent =
    `${description ? `${description} — ` : ""}${status} · solved pieces 0x${Number(info.solvedBits).toString(16).padStart(2, "0")}`;
}

function setBusy(buttons, cancelButton, status, busyText) {
  buttons.forEach((button) => { button.disabled = true; });
  cancelButton.classList.remove("hidden");
  status.textContent = busyText;
}

function setIdle(buttons, cancelButton, status, message) {
  buttons.forEach((button) => { button.disabled = false; });
  cancelButton.classList.add("hidden");
  status.textContent = message;
}

function makeResultCard(container, {
  success, title, moves = "", details = "", facelets = "", applyToSetup = true
}) {
  container.replaceChildren();
  container.classList.remove("hidden", "failure");
  if (!success) container.classList.add("failure");

  const heading = document.createElement("p");
  heading.className = "result-title";
  heading.textContent = title;
  container.append(heading);

  if (details) {
    const detail = document.createElement("p");
    detail.className = "result-copy";
    detail.textContent = details;
    container.append(detail);
  }
  if (moves) {
    const output = document.createElement("div");
    output.className = "move-output";
    output.textContent = moves;
    container.append(output);
    const copy = document.createElement("button");
    copy.className = "copy-button";
    copy.type = "button";
    copy.textContent = "Copy moves";
    copy.addEventListener("click", async () => {
      try {
        await navigator.clipboard.writeText(moves);
        copy.textContent = "Copied";
      } catch {
        copy.textContent = "Clipboard unavailable";
      }
    });
    container.append(copy);
    if (applyToSetup) {
      const apply = document.createElement("button");
      apply.className = "copy-button";
      apply.type = "button";
      apply.style.marginLeft = "7px";
      apply.textContent = "Apply to setup";
      apply.addEventListener("click", () => {
        currentSetup = [currentSetup, moves].filter(Boolean).join(" ");
        setupInput.value = currentSetup;
        inspectSetup();
      });
      container.append(apply);
    }
  }
  if (facelets) displayFacelets(facelets);
}

function numericValue(selector, label, min, max) {
  const input = document.querySelector(selector);
  const value = Number(input.value);
  if (!Number.isInteger(value) || value < min || value > max) {
    throw new Error(`${label} must be an integer from ${min} to ${max}`);
  }
  return value;
}

async function inspectSetup() {
  currentSetup = setupInput.value.trim();
  try {
    const info = await makeWorkerRequest("inspect", { moves: currentSetup });
    updateCubeInfo(info, "Setup applied");
    setEngineState("WebAssembly ready", "ready");
    return true;
  } catch (error) {
    setEngineState("Input error", "error");
    document.querySelector("#cube-status").textContent = error.message;
    return false;
  }
}

document.querySelector("#apply-setup").addEventListener("click", inspectSetup);
document.querySelector("#reset-setup").addEventListener("click", () => {
  currentSetup = "";
  setupInput.value = "";
  inspectSetup();
});
document.querySelector("#add-scramble").addEventListener("click", async () => {
  const button = document.querySelector("#add-scramble");
  button.disabled = true;
  try {
    const count = numericValue("#scramble-count", "Scramble length", 0, 1000);
    const scramble = await makeWorkerRequest("scramble", {
      count,
      seed: Date.now() >>> 0
    });
    currentSetup = [setupInput.value.trim(), scramble].filter(Boolean).join(" ");
    setupInput.value = currentSetup;
    await inspectSetup();
  } catch (error) {
    setEngineState("Input error", "error");
    document.querySelector("#cube-status").textContent = error.message;
  } finally {
    button.disabled = false;
  }
});

const goalMode = document.querySelector("#goal-mode");
function updateGoalControls() {
  const mode = goalMode.value;
  document.querySelector("#slot-control").classList.toggle("hidden", mode !== "slot");
  document.querySelector("#target-control").classList.toggle("hidden", mode !== "target" && mode !== "exact");
  const targetLabel = document.querySelector('label[for="target-moves"]');
  targetLabel.textContent = mode === "exact"
    ? "Target moves from the current setup"
    : "Target moves from solved (must leave F2L solved)";
}
goalMode.addEventListener("change", updateGoalControls);
updateGoalControls();

const searchButton = document.querySelector("#run-search");
const cancelSearchButton = document.querySelector("#cancel-search");
const searchStatus = document.querySelector("#search-status");
const searchResult = document.querySelector("#search-result");

searchButton.addEventListener("click", async () => {
  const mode = goalMode.value;
  const target = document.querySelector("#target-moves").value.trim();
  let payload;
  try {
    payload = {
      mode,
      setup: setupInput.value.trim(),
      target,
      slot: Number(document.querySelector("#slot-index").value),
      depth: numericValue("#max-depth", "Maximum depth", 0, 32),
      timeout: numericValue("#timeout", "Timeout", 0, 3600) * 1000,
      beam: numericValue("#beam-width", "Beam width", 1, 5000),
      pairPruning: document.querySelector("#pair-pruning").checked
    };
  } catch (error) {
    makeResultCard(searchResult, { success: false, title: "Invalid search options", details: error.message });
    return;
  }
  currentSetup = payload.setup;
  searchResult.classList.add("hidden");
  setBusy([searchButton], cancelSearchButton, searchStatus, "Searching in worker…");
  try {
    const result = await makeWorkerRequest("search", payload, () => {
      searchStatus.textContent = "Searching…";
    });
    if (!result.found) {
      makeResultCard(searchResult, {
        success: false,
        title: "No solution found",
        details: `No result within depth ${payload.depth} and ${payload.timeout / 1000}s. Try increasing the limits.`
      });
      setIdle([searchButton], cancelSearchButton, searchStatus, "Search complete");
      return;
    }
    const details = `${result.macroCount} macros · ${Number(result.nodes).toLocaleString()} nodes · ${result.verified ? "verified" : "verification failed"}`;
    makeResultCard(searchResult, {
      success: result.verified,
      title: result.verified ? "Verified solution" : "Unverified result",
      moves: result.moves,
      details: result.moves ? details : `${details} · already at goal`,
      facelets: result.finalFacelets
    });
    updateCubeInfo({
      facelets: result.finalFacelets,
      crossSolved: result.crossSolved,
      f2lSolved: result.f2lSolved,
      solvedBits: result.solvedBits
    }, "Search result");
    setIdle([searchButton], cancelSearchButton, searchStatus, "Search complete");
  } catch (error) {
    if (error.name === "AbortError") {
      setIdle([searchButton], cancelSearchButton, searchStatus, "Search cancelled");
      return;
    }
    makeResultCard(searchResult, { success: false, title: "Search failed", details: error.message });
    setIdle([searchButton], cancelSearchButton, searchStatus, "Check inputs");
  }
});

cancelSearchButton.addEventListener("click", () => {
  cancelWorker();
  setIdle([searchButton], cancelSearchButton, searchStatus, "Search cancelled");
});

function loadPreset() {
  document.querySelector("#recon-scramble").value = preset.scramble;
  document.querySelector("#recon-cross").value = preset.cross;
  document.querySelector("#recon-last-layer").value = preset.lastLayer;
}
document.querySelector("#load-preset").addEventListener("click", loadPreset);
loadPreset();

const reconstructButton = document.querySelector("#run-reconstruction");
const cancelReconButton = document.querySelector("#cancel-reconstruction");
const reconStatus = document.querySelector("#recon-status");
const reconResult = document.querySelector("#recon-result");

reconstructButton.addEventListener("click", async () => {
  let payload;
  try {
    payload = {
      scramble: document.querySelector("#recon-scramble").value.trim(),
      cross: document.querySelector("#recon-cross").value.trim(),
      lastLayer: document.querySelector("#recon-last-layer").value.trim(),
      depth: numericValue("#recon-depth", "Maximum depth", 0, 32),
      timeout: numericValue("#recon-timeout", "Search time", 0, 3600) * 1000,
      beam: numericValue("#recon-beam", "Beam width", 1, 5000)
    };
  } catch (error) {
    makeResultCard(reconResult, { success: false, title: "Invalid search options", details: error.message });
    return;
  }
  reconResult.classList.add("hidden");
  setBusy([reconstructButton], cancelReconButton, reconStatus, "Detecting orientation…");
  try {
    const result = await makeWorkerRequest("reconstruct", payload, () => {
      reconStatus.textContent = "Searching possible frames…";
    });
    if (!result.found) {
      makeResultCard(reconResult, {
        success: false,
        title: "No missing F2L found",
        details: `No result within depth ${payload.depth} and ${payload.timeout / 1000}s. Increase the search limits or check the supplied phases.`
      });
    } else {
      const frame = result.inspectionMoves || "identity";
      const compatibleFrames = result.compatibleInspections || "identity";
      const details = `Detected ${result.crossFace}-cross · inspection ${frame} (frame ${result.inspectionIndex + 1}/24) · compatible frames: ${compatibleFrames} · ${result.macroCount} macros · ${Number(result.nodes).toLocaleString()} nodes · verified through the last-layer algorithm`;
      makeResultCard(reconResult, {
        success: true,
        title: "Missing F2L recovered",
        moves: result.moves,
        details: result.moves ? details : `${details} · no missing moves`,
        facelets: result.finalFacelets,
        applyToSetup: false
      });
      document.querySelector("#frame-badge").textContent = `${result.crossFace} cross`;
      document.querySelector("#cube-status").textContent =
        `Detected inspection ${frame} (frame ${result.inspectionIndex + 1}/24) · cross and F2L complete before the supplied last layer.`;
    }
    setIdle([reconstructButton], cancelReconButton, reconStatus, "Search complete");
  } catch (error) {
    if (error.name === "AbortError") {
      setIdle([reconstructButton], cancelReconButton, reconStatus, "Search cancelled");
      return;
    }
    makeResultCard(reconResult, { success: false, title: "Reconstruction failed", details: error.message });
    setIdle([reconstructButton], cancelReconButton, reconStatus, "Check inputs");
  }
});

cancelReconButton.addEventListener("click", () => {
  cancelWorker();
  setIdle([reconstructButton], cancelReconButton, reconStatus, "Search cancelled");
});

document.querySelectorAll(".tab").forEach((tab) => {
  tab.addEventListener("click", () => {
    document.querySelectorAll(".tab").forEach((item) => item.classList.remove("active"));
    tab.classList.add("active");
    document.querySelectorAll(".tool-panel").forEach((panel) => panel.classList.add("hidden"));
    document.querySelector(`#${tab.dataset.panel}`).classList.remove("hidden");
  });
});

inspectSetup();
