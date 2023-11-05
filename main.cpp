# include <Siv3D.hpp> // OpenSiv3D v0.6.12

// シーンの名前
enum class State
{
	Title,
	SandCastle,
	Ranking,
};

// 共有するデータ
struct GameData
{
	// 直前のゲームのスコア
	Optional<int32> lastGameScore;

	// ハイスコア
	Array<int32> highScores = { 0, 0, 0, 0, 0 };

	Grid<int32> sandmap;
};

using App = SceneManager<State, GameData>; //Scene管理

// タイトルシーン
class Title : public App::Scene
{
public:

	Title(const InitData& init)
		: IScene{ init }
	{
	}


	~Title()
	{
	}


	void update() override
	{
		AudioAsset(U"BGM").play();
		if (SimpleGUI::Slider(volume, Vec2{ 50, 600 }, 200, 300))
		{
			// 音量を設定
			AudioAsset(U"BGM").setVolume(volume);
		}

		m_startTransition.update(m_startButton.mouseOver());
		m_rankingTransition.update(m_rankingButton.mouseOver());
		m_exitTransition.update(m_exitButton.mouseOver());

		if (m_startButton.mouseOver() || m_rankingButton.mouseOver() || m_exitButton.mouseOver())
		{
			Cursor::RequestStyle(CursorStyle::Hand);
		}



		if (m_startButton.leftClicked())
		{
			// ゲームシーンへ
			changeScene(State::SandCastle);
		}
		else if (m_rankingButton.leftClicked())
		{
			// ランキングシーンへ
			changeScene(State::Ranking);
		}
		else if (m_exitButton.leftClicked())
		{
			// 終了
			System::Exit();
		}
	}

	//描写だけ
	void draw() const override
	{
		//背景
		Scene::Rect().draw(Arg::top = ColorF{ 0.2, 0.5, 1.0 }, Arg::bottom = ColorF{ 0.5, 0.8, 1.0 });

		FontAsset(U"TitleFont")(U"Sand Castle").drawAt(100, Vec2{ 400, 100 });

		m_startButton.draw(ColorF{ 1.0, m_startTransition.value() }).drawFrame(2);
		m_rankingButton.draw(ColorF{ 1.0, m_rankingTransition.value() }).drawFrame(2);
		m_exitButton.draw(ColorF{ 1.0, m_exitTransition.value() }).drawFrame(2);

		FontAsset(U"Menu")(U"PLAY").drawAt(m_startButton.center(), ColorF{ 0.25 });
		FontAsset(U"Menu")(U"RANKING").drawAt(m_rankingButton.center(), ColorF{ 0.25 });
		FontAsset(U"Menu")(U"EXIT").drawAt(m_exitButton.center(), ColorF{ 0.25 });

	}

private:

	Rect m_startButton{ Arg::center = Scene::Center(), 300, 60 };
	Transition m_startTransition{ 0.4s, 0.2s };

	Rect m_rankingButton{ Arg::center = Scene::Center().movedBy(0, 100), 300, 60 };
	Transition m_rankingTransition{ 0.4s, 0.2s };

	Rect m_exitButton{ Arg::center = Scene::Center().movedBy(0, 200), 300, 60 };
	Transition m_exitTransition{ 0.4s, 0.2s };

	double volume = 1.0;

};


class SandCastle : public App::Scene
{
public:
	SandCastle(const InitData& init)
		: IScene{ init }
	{
		//砂場領域
		getData().sandmap.resize(m_height, m_width, 0);

		//スコップの設定
		scoops << Scoop{ Scoopname::Mini, Texture{U"NewAssets/photo/mini.png"}, U"1マスだけ掘れるよ", 1 };
		scoops << Scoop{ Scoopname::Standard, Texture{U"NewAssets/photo/standard.png"}, U"4マス掘れるよ", 2 };
		scoops << Scoop{ Scoopname::Miduim, Texture{U"NewAssets/photo/medium.png"}, U"9マス掘れるよ", 3 };
		scoops << Scoop{ Scoopname::Big, Texture{U"NewAssets/photo/mini.png"}, U"25マス掘れるよ", 4 };

		//実装が間に合わなかった
		//scoops << Scoop{ Scoopname::Cross, Texture{U"NewAssets/photo/mini.png"}, U"十字に5マス掘れるよ", 4 };
		//scoops << Scoop{ Scoopname::triangle, Texture{U"NewAssets/photo/mini.png"}, U"三角に6マス掘れるよ", 4 };

		//初期スコップの選択&アイテム欄に追加
		l_scoop = scoops[0]; items << Item{ l_scoop, l_scoop_rect,false,0};
		c_scoop = scoops[1]; items << Item{ c_scoop, c_scoop_rect,false,1 };
		r_scoop = scoops[2]; items << Item{ r_scoop, r_scoop_rect,false,2};

	}

	//ここから更新関数
	void update() override
	{

		//スコップ選択
		for (auto& x : items) {
			if (x.rect.leftClicked())
			{

				if (x.ok==true)
				{
					isChoiseitem = false;
					x.ok = false;
					nowindex = 99;
				}

				else {
					nowitem = x.scoop;
					x.ok = true;
					isChoiseitem = true;
					nowindex = x.nowindex;
				}
				//Print << nowitem.scoop_description;
			}
		}

		//グリッド操作
		for (int32 y = 0; y < m_height; y++) {
			for (int32 x = 0; x < m_width; x++) {

				//クリック領域の定義
				const RectF rect{ (Point{ (x * m_size), (y * m_size)} + Offset), m_size };

				//アイテムが選択されるとき
				if (isChoiseitem)
				{
					//はみ出さない上限値
					m_x = Min(x, m_width - nowitem.lenth);
					m_y = Min(y, m_width - nowitem.lenth);
					//スコップがマップの範囲外になったときに、参照をずらす
					
					const RectF scooprect{ (Point{Min(x,m_width - nowitem.lenth)*m_size,Min(y,m_height - nowitem.lenth)*m_size}+Offset),m_size };


					//掘る範囲が決定されたとき
					if (rect.leftClicked())
					{
						int32 countall = 0;
						int32 countone=100;
						//配列の値を変える

						for (int32 i = m_y; i < m_y + nowitem.lenth; i++) {
							for (int32 j = m_x; j < m_x + nowitem.lenth; j++) {

								//周囲のマスを調べて、深さの差が2以上であるなら、掘り進めることはできない（無限に掘れる）
								//今から実装する

								countall+= getData().sandmap[i][j];
								countone = Min(countone,getData().sandmap[i][j]);
							}
						}
						bool overlap = false;
						if (countall / (nowitem.lenth) == countone) {
							overlap = true;
						}
						for (int32 i = m_y; i < m_y + nowitem.lenth; i++) {
							for (int32 j = m_x; j < m_x + nowitem.lenth; j++) {
								getData().sandmap[i][j] = countone + true;
								m_score += 10 * Pow(getData().sandmap[i][j], 2);
							}
						}

						m_turn--;
						int32 indexcount = 0;
						for(auto& x : items)
						{
							if (x.ok = true) {

								items[indexcount] = Item(scoops.choice(), items[indexcount].rect, false);
							}
							indexcount++;
						}
					}


				}

			}
		}

		//turn終了したらランキングへ
		if(m_turn<=0)
		{
			// ランキング画面へ
			changeScene(State::Ranking);

			getData().lastGameScore = m_score;

		}


	}

	//描写
	void draw() const override
	{
		//背景
		Scene::Rect().draw(Arg::top = ColorF{ 0.2, 0.5, 1.0 }, Arg::bottom = ColorF{ 0.5, 0.8, 1.0 });

		//砂マップ
		for (int32 y = 0; y < m_height; y++) {
			for (int32 x = 0; x < m_width; x++) {
				const RectF rect{ (Point{ (x * m_size), (y * m_size)} + Offset), m_size };
				const ColorF color{ (3 - getData().sandmap[y][x]) / 3.0 };
				rect.stretched(-1).draw(color);
				if ((isChoiseitem==true) && rect.mouseOver())
				{
					rect.drawFrame(8, 0);
					
					const RectF scooprect{ (Point{Min(x,m_width - nowitem.lenth)*m_size,Min(y,m_height - nowitem.lenth)*m_size}+Offset),m_size*nowitem.lenth };
					scooprect.draw(color);

				}

			}
		}

		//スコップ（アイテム）欄
		inv_rect.rounded(10).draw(inv_color);
		inv_rect.rounded(10).drawFrame(3, 3, ColorF{ 0.5 });

		//スコップ
		for (auto& x : items) {
			x.rect.draw(Palette::Whitesmoke);
			if(x.ok)
			{
				x.rect.drawFrame(5, 0);
			}
			x.scoop.texture.draw(x.rect.x+20,x.rect.y+20);
		}

		//アイテム説明
		//選択アイテムの説明をする
		if (isChoiseitem)
		{
				FontAsset(U"Text")(nowitem.description).drawAt(30, Vec2{ 1000, 600 });
				//Print << nowitem.scoop_description;
			
		}

		//マウスカーソルが重なったアイテムの説明もする
		for(auto& x: items)
		{
			if ((isChoiseitem==false) && x.rect.mouseOver())
			{
				FontAsset(U"Text")(x.scoop.description).drawAt(30, Vec2{ 1000, 600 });
				//Print << nowitem.scoop_description;
			}
		}

		//残りターン数の表示
		FontAsset(U"Text")(U"日暮れまであと\n{}ターン"_fmt(m_turn)).drawAt(30, Vec2{1000,80});

		//スコアの表示
		FontAsset(U"Text")(U"現在のスコア　{}"_fmt(m_score)).drawAt(30, Vec2{1000,150});
		
	}

private:

	//フラグ管理
	bool flag = false;

	//sandmapサイズ
	constexpr static int32 m_width = 9;
	constexpr static int32 m_height = 9;
	constexpr static int32 m_size = 50;

	constexpr static Point Offset{ 80, 70 };

	constexpr static Rect LastOffset{ 600, 600 };

	//スコップが範囲外を選択しないように、みせかけのx,yを設定する
	int32 m_x = 100;
	int32 m_y = 100;


	//土の種類

	enum class Scoopname
	{
		Mini,
		Standard,
		Miduim,
		Big,
		Cross,
		Triangle,
	};

	struct Scoop {
		Scoopname name;       // 砂の名前
		Texture texture;       // 砂の色
		String description;// 砂の説明文
		int32 lenth=0;   // 砂の硬さ（掘るのに必要な労力などに影響するかも）
	};

	int32 scoop_size = 4;
	Array<Scoop> scoops;

	//アイテム欄(inventory)
	const RectF inv_rect{ 75, 600, 600, 100 };
	const ColorF inv_color{ 0.8,0.8,0.8 };

	//今選んでるアイテム
	Scoop nowitem;
	//今選んでるのがどれか
	int32 nowindex;

	//アイテムを選んでいるか
	bool isChoiseitem=false;
	//アイテムが選ばれているか
	bool isCoosen = false;

	//アイテム欄のスコップ3種類

	struct Item
	{
		Scoop scoop;
		RectF rect;
		bool ok;
		int32 nowindex;
	};

	//アイテムの配列
	Array<Item> items;

	Scoop l_scoop, c_scoop, r_scoop;
	const RectF l_scoop_rect{ Arg::center(150,650),85 };
	const RectF c_scoop_rect{ Arg::center(300,650),85 };
	const RectF r_scoop_rect{ Arg::center(450,650),85 };

	//アイテム説明欄
	const RectF discription{ Arg::center(800,600),100 };


	//ターン数
	int32 m_turn=15;

	//ゲームのスコア
	int32 m_score = 0;

};


// ランキングシーン
class Ranking : public App::Scene
{
public:

	Ranking(const InitData& init)
		: IScene{ init }
	{
		auto& data = getData();

		if (data.lastGameScore)
		{
			const int32 lastScore = *data.lastGameScore;

			// ランキングを再構成
			data.highScores << lastScore;
			data.highScores.rsort();
			data.highScores.resize(RankingCount);

			// ランクインしていたら m_rank に順位をセット
			for (int32 i = 0; i < RankingCount; ++i)
			{
				if (data.highScores[i] == lastScore)
				{
					m_rank = i;
					break;
				}
			}

			data.lastGameScore.reset();
		}
	}

	void update() override
	{
		if (MouseL.down())
		{
			// タイトルシーンへ
			changeScene(State::Title);
		}
	}

	void draw() const override
	{
		Scene::SetBackground(ColorF{ 0.4, 0.6, 0.9 });

		FontAsset(U"Ranking")(U"RANKING").drawAt(400, 60);

		const auto& data = getData();

		// ランキングを表示
		for (auto i : step(RankingCount))
		{
			const RectF rect{ 100, 120 + i * 80, 600, 80 };

			rect.draw(ColorF{ 1.0, 1.0 - i * 0.2 });

			FontAsset(U"Ranking")(data.highScores[i]).drawAt(rect.center(), ColorF{ 0.25 });

			// ランクインしていたら
			if (i == m_rank)
			{
				rect.stretched(Periodic::Triangle0_1(0.5s) * 10).drawFrame(10, ColorF{ 0.8, 0.6, 0.4 });
			}
		}
	}

private:

	static constexpr int32 RankingCount = 5;

	int32 m_rank = -1;
};



void Main()
{
	Window::SetTitle(U"Sand Castels");
	//Windowサイズ変更
	Window::Resize(1280, 720);
	// 背景色を RGB で指定する
	Scene::SetBackground(ColorF{ 0.098, 0.443, 0.890 });
	//タイトルフォントの設定
	FontAsset::Register(U"TitleFont", FontMethod::MSDF, 50, U"NewAssets/font/JF-Dot-MPlus12B.ttf");
	FontAsset(U"TitleFont").setBufferThickness(4);
	FontAsset::Register(U"Text", FontMethod::MSDF, 20, U"NewAssets/font/JF-Dot-MPlus12B.ttf");
	FontAsset::Register(U"Menu", FontMethod::MSDF, 40, Typeface::Medium);
	FontAsset::Register(U"Ranking", 40, Typeface::Heavy);
	FontAsset::Register(U"GameScore", 30, Typeface::Light);
	AudioAsset::Register(U"Brick", GMInstrument::Woodblock, PianoKey::C5, 0.2s, 0.1s);

	//曲の設定
	AudioAsset::Register(U"BGM", Audio::Stream, U"NewAssets/buckmusic.wav", Loop::Yes);
	double volume = 1.0;

	//わけわかめ　もうなにもわからない
	// 
	// シーンマネージャーを作成
	App manager;

	// タイトルシーンを登録
	manager.add<Title>(State::Title);
	manager.add<SandCastle>(State::SandCastle);
	manager.add<Ranking>(State::Ranking);

	// "Game" シーンから開始
	//manager.init(State::SandCastle);

	while (System::Update())
	{
		// 現在のシーンを実行
		// シーンに実装した .update() と .draw() が実行される
		if (not manager.update())
		{
			break;
		}

	}
}
