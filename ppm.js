function save_canvas_to_ppm_file(canvas, filename = 'canvas.ppm') {
	const ctx = canvas.getContext('2d');
	const { width, height } = canvas;

	const header = new TextEncoder().encode(`P6\n${width} ${height} 255\n`);
	const rgba_data = ctx.getImageData(0, 0, width, height).data;

	// file_data is RGB, so it requires only 3 bytes per pixel
	const file_data = new Uint8Array(header.length + rgba_data.length / 4 * 3);
	file_data.set(header, 0);

	let j = 0;
	for (let i = 0; i < rgba_data.length; ++i) {
		if (i % 4 == 3) continue; // skip alpha byte
		file_data[header.length + j++] = rgba_data[i];
	}

	const a = document.createElement('a');
	a.href = URL.createObjectURL(new Blob([file_data], { type: 'application/octet-stream' }));
	a.download = filename;
	a.style.visibility = 'none';
	document.body.appendChild(a);
	a.click();
	document.body.removeChild(a);
}
