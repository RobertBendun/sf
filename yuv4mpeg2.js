class Yuv4mpeg2 {
	constructor(width, height, FPS) {
		if (Math.floor(FPS) !== FPS) {
			throw new Error('FPS must be an integer!');
		}
		Object.assign(this, {
			width, height, FPS,
			frames: [],
			frame_header: new TextEncoder().encode('FRAME\n'),
		});
	}

	push_frame_from_canvas(canvas) {
		function rgb_to_ycrcb(r, g, b) {
			return [
				(16  +  65.738*r/256 + 129.057*g/256 +  25.064*b/256),
				(128 -  37.945*r/256 -  74.494*g/256 + 112.439*b/256),
				(128 + 112.439*r/256 -  94.154*g/256 -  18.285*b/256),
			];
		}

		const ctx = canvas.getContext('2d');
		const { width, height } = canvas;
		if (width != this.width || height != this.height)
			throw new Error(`Expected (width = ${this.width}, height = ${this.height}. Got (${width}, ${height})`);

		const rgba_data = ctx.getImageData(0, 0, width, height).data;

		const buffers = [0, 0, 0].map(_ => new Uint8Array(rgba_data.length / 4));

		const RGB = [0, 0, 0];
		for (let i = 0, j = 0; i < rgba_data.length; ++i) {
			let index = i % 4;
			if (index == 3) {
				const [y, cr, cb] = rgb_to_ycrcb(...RGB);
				buffers[0][j]   = y;
				buffers[2][j]   = cb;
				buffers[1][j++] = cr;
				continue;
			}
			RGB[index] = rgba_data[i];
		}

		this.frames.push(this.frame_header);
		this.frames.push(...buffers);
	}

	save(filename = 'canvas.y4m') {
		console.log('saving ' + filename)
		const file_header = new TextEncoder().encode(`YUV4MPEG2 W${this.width} H${this.height} F${this.fps}:1 Ip A1:1 C444\n`);
		this.frames.unshift(file_header);

		const a = document.createElement('a');
		a.href = URL.createObjectURL(new Blob(this.frames, { type: 'application/octet-stream' }));
		a.download = filename;
		a.style.visibility = 'none';
		document.body.appendChild(a);
		a.click();
		document.body.removeChild(a);

		this.frames = [];
	}

}
