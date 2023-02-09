#include "render.h"

int main()
{

	std::vector<std::string> checks;

	checks = getChecks("GameLevel.txt");

	Level level;

	for (size_t i = 0; i < checks.size(); i++)
	{
		Model object("GameLevel.txt", checks[i]);
		object.matOffset = level.getMaterialOffset();
		object.UpdateBatch();
		level.addIndices(object.BuildIndicies());
		level.addVertices(object.BuildVertices());
		level.addModel(object);
	}
	// Get info
	/*
	Model sink("GameLevel.txt", "Kitchen_Sink");

	sink.matOffset = level.getMaterialOffset();
	sink.UpdateBatch();
	level.addIndices(sink.BuildIndicies());
	level.addVertices(sink.BuildVertices());
	level.addModel(sink);

	Model breh("GameLevel.txt", "Chair_4");

	breh.matOffset = level.getMaterialOffset();
	breh.UpdateBatch();
	level.addIndices(breh.BuildIndicies());
	level.addVertices(breh.BuildVertices());
	level.addModel(breh);
	*/
	// Create Window and Render
	GWindow win;
	GEventResponder msgs;
	GOpenGLSurface ogl;
	if (+win.Create(0, 0, 800, 600, GWindowStyle::WINDOWEDBORDERED)) {
		float clr[] = { 110 / 255.0f, 70 / 255.0f , 67 / 255.0f, 1 };
		win.SetWindowName("Jam Jam's Adventure Island 2.5 Re:Written");
		msgs.Create([&](const GW::GEvent& e) {
			GW::SYSTEM::GWindow::Events q;
			if (+e.Read(q) && q == GWindow::Events::RESIZE) {
				clr[2] += 0.01f;
			}
		});
		win.Register(msgs);
		if (+ogl.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT)) {
			Render boot(win, ogl, level);
			while (+win.ProcessWindowEvents())
			{
				boot.UpdateCamera();
				glClearColor(clr[0], clr[1], clr[2], clr[3]);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				boot.render();
				ogl.UniversalSwapBuffers();
			}
		}
	}

	return 0;
}