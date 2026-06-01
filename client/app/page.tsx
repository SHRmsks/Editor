"use client";
import { useEffect, useRef, memo } from "react";
import {
  vertexShaderSource,
  fragmentShaderSource,
  compileShader,
  createProgram,
  CursorVertexShaderSource,
  CursorFragmentShaderSource,
} from "@/lib/shader";

const Main = () => {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const hiddenInput = useRef<HTMLInputElement>(null);
  const clibRef = useRef<any>(null); // Quick any type for the wasm exports
  const scrollY = useRef<number | null>(0);
  // the cursor tracker
  const cursorOffset = useRef<number | null>(0);
  const isTyping = useRef<boolean | null>(false);
  const needsUpdate = useRef<boolean | null>(true);
  const needsUpdateCursor = useRef<boolean | null>(true);
  const CursorX = useRef<number | null>(0);
  const CursorY = useRef<number | null>(0);
  const blinkTimer = useRef<number | null>(Date.now());
  const FONT_PX = 20;
  const LINE_H = 1.5 * FONT_PX;
  // 1. Initial Loading
  useEffect(() => {
    const WASMloader = async () => {
      // Load the WASM module
      const [wasmResponse, fontResponse] = await Promise.all([
        fetch("/clib.wasm"),
        fetch("/custom.json"),
      ]);
      if (!wasmResponse.ok || !fontResponse.ok) {
        console.error("Failed to load WASM module");
        return;
      }
      const font = await fontResponse.json();
      const atlasW = font.atlas.width;
      const atlasH = font.atlas.height;
      /*  DEBUG  */
      // console.log("font width and height", atlasW, atlasH);
      const env = {
        env: {
          console_log_int: (value: number) => {
            console.log(value);
          },
          console_log_string: (value: string) => {
            console.log(value);
          },
        },
      };

      try {
        const instance = await WebAssembly.instantiateStreaming(
          wasmResponse,
          env,
        );
        const clib = instance.instance.exports;
        // Save the WebAssembly exports to our ref
        clibRef.current = clib;
        console.log("WASM module loaded successfully");
        //load the json and png file
        for (const glyph of font.glyphs) {
          const id = glyph.unicode;
          const adv = glyph.advance || 0;
          const pl = glyph.planeBounds ? glyph.planeBounds.left : 0;
          const pb = glyph.planeBounds ? glyph.planeBounds.bottom : 0;
          const pr = glyph.planeBounds ? glyph.planeBounds.right : 0;
          const pt = glyph.planeBounds ? glyph.planeBounds.top : 0;
          const al = (glyph.atlasBounds ? glyph.atlasBounds.left : 0) / atlasW;
          const ab =
            (glyph.atlasBounds ? glyph.atlasBounds.bottom : 0) / atlasH;
          const ar = (glyph.atlasBounds ? glyph.atlasBounds.right : 0) / atlasW;
          const at = (glyph.atlasBounds ? glyph.atlasBounds.top : 0) / atlasH;
          clibRef.current.load_glyph_metric(
            id,
            adv,
            pl,
            pb,
            pr,
            pt,
            al,
            ab,
            ar,
            at,
          );
        }
        clibRef.current.init_document(0, 0);
        window.dispatchEvent(new Event("resize"));
      } catch (e) {
        console.error("Error instantiating WASM module:", e);
      }
    };
    WASMloader(); // Call the wasm setup loader
  }, []); // run immediately
  // 2. Input Handler
  const inputHandler = (e: React.ChangeEvent<HTMLInputElement>) => {
    if (!clibRef.current) return;
    const txt = e.target.value;
    if (!txt) return;
    const encoder = new TextEncoder();
    const encoded = encoder.encode(txt);
    // Read the memory directly from C
    const bufptr = clibRef.current.getScratchBuffer();
    const wasmMemory = new Uint8Array(clibRef.current.memory.buffer);
    wasmMemory.set(encoded, bufptr); // copy the ecoded buffer values to the wasm memory
    // DEBUG:
    // console.log("cursor_offset", cursorOffset.current);
    clibRef.current.insert_text(bufptr, encoded.length, cursorOffset.current);
    if (cursorOffset.current !== null) {
      cursorOffset.current! += txt.length;
    }
    // console.log("values", hiddenInput.current!.value);
    if (hiddenInput.current) hiddenInput.current.value = ""; // clear out
    needsUpdate.current = true;
    isTyping.current = true;
    needsUpdateCursor.current = true;
    blinkTimer.current = Date.now();
  };
  const keydownHandler = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (!clibRef.current) return;
    if (e.key === "Backspace" && cursorOffset.current! > 0) {
      // console.log("Backspace at offset", cursorOffset.current);
      clibRef.current.delete_text(cursorOffset.current! - 1, 1);
      if (cursorOffset.current !== null) {
        cursorOffset.current = Math.max(0, cursorOffset.current - 1);
      }
      needsUpdate.current = true;
      blinkTimer.current = Date.now();
      needsUpdateCursor.current = true;
      isTyping.current = true;
    } else if (
      e.key === "ArrowLeft" &&
      cursorOffset.current !== null &&
      cursorOffset.current > 0
    ) {
      cursorOffset.current--;
      isTyping.current = true;
      needsUpdateCursor.current = true;
      blinkTimer.current = Date.now();
    } else if (e.key === "ArrowRight" && cursorOffset.current !== null) {
      cursorOffset.current += 1;
      isTyping.current = true;
      needsUpdateCursor.current = true;
      blinkTimer.current = Date.now();
    } else if (e.key == "ArrowUp") {
      e.preventDefault(); // prevent the default scrolling behavior
      // get the current cursor x and y from the render module
      const currentX = clibRef.current.get_current_cursor_x();
      const currentY = clibRef.current.get_current_cursor_y();
      // move up by LINE_H
      const targetY =
        currentY -
        LINE_H * (window.devicePixelRatio ? window.devicePixelRatio : 1);
      // update the cursor VBO with the new target Y and the same X
      cursorOffset.current = clibRef.current.update_cursor_VBO(
        currentX,
        targetY,
        scrollY.current,
      );
      needsUpdateCursor.current = true;
      isTyping.current = true;
      blinkTimer.current = Date.now();
    } else if (e.key == "ArrowDown") {
      e.preventDefault(); // prevent the default scrolling behavior
      // get the current cursor x and y from the render module
      const currentX = clibRef.current.get_current_cursor_x();
      const currentY = clibRef.current.get_current_cursor_y();
      // move up by LINE_H
      const targetY = currentY + LINE_H * window.devicePixelRatio;
      // update the cursor VBO with the new target Y and the same X
      cursorOffset.current = clibRef.current.update_cursor_VBO(
        currentX,
        targetY,
        scrollY.current,
      );
      needsUpdateCursor.current = true;
      isTyping.current = true;
      blinkTimer.current = Date.now();
    } else if (e.key === "Enter") {
      e.preventDefault(); // prevent the default behavior of adding a newline in the input
      const encoder = new TextEncoder();
      const encoded = encoder.encode("\n");
      const bufptr = clibRef.current.getScratchBuffer();
      const wasmMemory = new Uint8Array(clibRef.current.memory.buffer);
      wasmMemory.set(encoded, bufptr);
      clibRef.current.insert_text(bufptr, encoded.length, cursorOffset.current);
      if (cursorOffset.current !== null) {
        cursorOffset.current! += 1; // move cursor forward by 1 for the newline
      }
      if (hiddenInput.current) hiddenInput.current.value = "";
      needsUpdate.current = true;
      isTyping.current = true;
      needsUpdateCursor.current = true;
      blinkTimer.current = Date.now();
    } else if (e.key == "Tab") {
      e.preventDefault();
      const encoder = new TextEncoder();
      const encoded = encoder.encode("\t");
      const bufptr = clibRef.current.getScratchBuffer();
      const wasmMemory = new Uint8Array(clibRef.current.memory.buffer);
      wasmMemory.set(encoded, bufptr);
      clibRef.current.insert_text(bufptr, encoded.length, cursorOffset.current);
      if (cursorOffset.current !== null) {
        cursorOffset.current! += 1; // move cursor forward by 2 for the tab
      }
      if (hiddenInput.current) hiddenInput.current.value = "";
      needsUpdate.current = true;
      isTyping.current = true;
      needsUpdateCursor.current = true;
      blinkTimer.current = Date.now();
    }
  };
  // 3. Render Loop
  useEffect(() => {
    if (!canvasRef.current) return;
    const ctx = canvasRef.current.getContext("webgl2", {
      alpha: false, // we control through shader
      antialias: false,
    });
    if (!ctx) return;
    const vertexShader = compileShader(
      ctx,
      ctx.VERTEX_SHADER, // vertex shader
      vertexShaderSource,
    );
    const fragmentShader = compileShader(
      ctx,
      ctx.FRAGMENT_SHADER, // frag shader
      fragmentShaderSource,
    );
    const cursorVertexShader = compileShader(
      ctx,
      ctx.VERTEX_SHADER, // vertex shader
      CursorVertexShaderSource,
    );
    const cursorFragmentShader = compileShader(
      ctx,
      ctx.FRAGMENT_SHADER, // frag shader
      CursorFragmentShaderSource,
    );
    // PROGRAM SETUP
    const program = createProgram(ctx, vertexShader, fragmentShader);
    const cursorProgram = createProgram(
      ctx,
      cursorVertexShader,
      cursorFragmentShader,
    );
    const positionID = 0;
    const textureID = 1;
    // BUFFER SETUP
    const VBO = ctx.createBuffer();
    const cursorVBO = ctx.createBuffer();

    const texture = ctx.createTexture();
    const image = new Image();
    image.src = "/custom.png";
    image.onload = () => {
      ctx.bindTexture(ctx.TEXTURE_2D, texture);
      ctx.pixelStorei(ctx.UNPACK_FLIP_Y_WEBGL, true); // flip the y-axis to match the texture coordinates
      // ??
      ctx.texImage2D(
        ctx.TEXTURE_2D,
        0,
        ctx.RGBA,
        ctx.RGBA,
        ctx.UNSIGNED_BYTE,
        image,
      );
      ctx.texParameteri(ctx.TEXTURE_2D, ctx.TEXTURE_WRAP_S, ctx.CLAMP_TO_EDGE);
      ctx.texParameteri(ctx.TEXTURE_2D, ctx.TEXTURE_WRAP_T, ctx.CLAMP_TO_EDGE);
      ctx.texParameteri(ctx.TEXTURE_2D, ctx.TEXTURE_MIN_FILTER, ctx.LINEAR);
      ctx.texParameteri(ctx.TEXTURE_2D, ctx.TEXTURE_MAG_FILTER, ctx.LINEAR);
    };
    ctx.enable(ctx.BLEND);
    ctx.blendFunc(ctx.SRC_ALPHA, ctx.ONE_MINUS_SRC_ALPHA);
    const resolutionUniformLoc = ctx.getUniformLocation(program, "resolution");
    const cursorResLoc = ctx.getUniformLocation(cursorProgram, "resolution");

    let animationID: number;
    const resize = () => {
      const dpr = window.devicePixelRatio || 1;
      const cssW = canvasRef.current!.clientWidth,
        cssH = canvasRef.current!.clientHeight;
      const PAD_X = 40 * dpr;
      const PAD_Y = 40 * dpr;
      const w = Math.round(cssW * dpr);
      const h = Math.round(cssH * dpr);
      if (canvasRef.current!.width !== w) canvasRef.current!.width = w;
      if (canvasRef.current!.height !== h) canvasRef.current!.height = h;
      ctx.viewport(0, 0, w, h);
      clibRef.current?.update_config(
        w,
        h,
        LINE_H * dpr,
        FONT_PX * dpr,
        PAD_X,
        PAD_Y,
      );
      ctx.useProgram(program);
      ctx.uniform2f(resolutionUniformLoc, w, h);
      ctx.useProgram(cursorProgram);
      ctx.uniform2f(cursorResLoc, w, h);
      needsUpdate.current = true;
    };
    resize();
    window.addEventListener("resize", resize);
    let VBOCount = 0;
    const render = () => {
      if (clibRef.current) {
        // Clear the screen
        ctx.clearColor(1.0, 1.0, 1.0, 1.0);
        ctx.clear(ctx.COLOR_BUFFER_BIT);
        //  generate the frame
        // Phae 1:
        // the TEXT and bind to the context buffer
        ctx.useProgram(program);
        ctx.bindBuffer(ctx.ARRAY_BUFFER, VBO);
        ctx.enableVertexAttribArray(positionID);
        ctx.enableVertexAttribArray(textureID);
        // total 4 floats per vertex
        ctx.vertexAttribPointer(positionID, 2, ctx.FLOAT, false, 16, 0); //start at 0 relative offset
        ctx.vertexAttribPointer(textureID, 2, ctx.FLOAT, false, 16, 8);
        if (needsUpdate.current) {
          clibRef.current.generate_VBO_frame(scrollY.current);
          VBOCount = clibRef.current.get_vbo_count();
          const VBOPtr = clibRef.current.get_vbo_buffer();
          if (VBOCount > 0) {
            const vboArray = new Float32Array(
              clibRef.current.memory.buffer,
              VBOPtr,
              VBOCount,
            );
            ctx.bufferData(ctx.ARRAY_BUFFER, vboArray, ctx.STATIC_DRAW);
          }
          needsUpdate.current = false;
        }
        if (VBOCount > 0) {
          ctx.drawArrays(ctx.TRIANGLES, 0, VBOCount / 4);
        }
        // Phase 2:
        // the CURSOR
        // pick the cursor Program
        ctx.useProgram(cursorProgram);
        ctx.bindBuffer(ctx.ARRAY_BUFFER, cursorVBO);
        ctx.disableVertexAttribArray(1);
        ctx.enableVertexAttribArray(0);
        ctx.vertexAttribPointer(0, 2, ctx.FLOAT, false, 8, 0);

        if (needsUpdateCursor.current) {
          if (isTyping.current) {
            clibRef.current.update_cursor_index_VBO(cursorOffset.current); // to get the latest cursor position after typing
            isTyping.current = false;
          } else {
            cursorOffset.current = clibRef.current.update_cursor_VBO(
              CursorX.current,
              CursorY.current,
            );
          }

          const CursorVBOPtr = clibRef.current.get_CURSOR_VBO();
          const cursorVBOArray = new Float32Array(
            clibRef.current.memory.buffer,
            CursorVBOPtr,
            12, // 5 vertices, each with 2 floats (x,y)
          );
          ctx.bufferData(ctx.ARRAY_BUFFER, cursorVBOArray, ctx.STATIC_DRAW);
          needsUpdateCursor.current = false;
        }

        const now = Date.now();
        if (
          blinkTimer.current &&
          (now - blinkTimer.current < 500 ||
            Math.floor((now - blinkTimer.current) / 500) % 2 === 0)
        ) {
          ctx.drawArrays(ctx.TRIANGLES, 0, 6); // two triangles for the cursor
        }
      }
      animationID = requestAnimationFrame(render);
    };

    requestAnimationFrame(render);
    return () => cancelAnimationFrame(animationID);
  }, []);

  return (
    <div
      className="w-screen h-screen bg-gray-100 relative overflow-hidden"
      onClick={() => hiddenInput.current?.focus()}
    >
      <canvas
        className="bg-white w-full h-full absolute top-0 left-0"
        ref={canvasRef}
        width={1000} // Set explicit canvas coordinate sizes
        height={1000}
        onClick={(e) => {
          if (canvasRef.current) {
            const rec = canvasRef.current.getBoundingClientRect();
            const dpr = window.devicePixelRatio || 1;
            CursorX.current = (e.clientX - rec.left) * dpr;
            CursorY.current = (e.clientY - rec.top) * dpr + scrollY.current!;
            needsUpdateCursor.current = true;
            isTyping.current = false;
            needsUpdate.current = true;
            blinkTimer.current = Date.now();
          }
        }}
        onWheel={(e) => {
          scrollY.current = Math.max(
            0,
            scrollY.current! + e.deltaY * window.devicePixelRatio
              ? window.devicePixelRatio
              : 1,
          );
          console.log("scrollY", scrollY.current);
          needsUpdate.current = true;
        }}
      ></canvas>
      <input
        type="text"
        ref={hiddenInput}
        onChange={inputHandler}
        onKeyDown={keydownHandler}
        onBlur={() => hiddenInput.current?.focus()}
        className="absolute top-0 left-0 opacity-0 w-[1px] h-[1px]"
      />
    </div>
  );
};

export default memo(Main);
