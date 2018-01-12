#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

constexpr unsigned int windowWidth{ 800 }, windowHeight{ 600 };
constexpr float ballRadius{ 10.f }, ballVelocity{ 7.f };

constexpr float rectangleWidth{ 80.f }, rectangleHeigth{ 20.f }, rectangleVelocity{ 9.f };

constexpr float blockWidth{ 60.f }, blockHeight{ 20.f };
constexpr int countBlocksX{ 11 }, countBlocksY{ 5 }; //ammount of bricks

Font font;
int lives = 3;
bool not_moving{ true };
// We will use an `std::vector` to contain any number
// of `Brick` instances.
struct Ball
{
	// CircleShape is an SFML class that
	// defines a renderable circle.
	CircleShape shape;

	//Vector of ball's speed
	Vector2f velocity{ 0, 0 };

	//Ball constructor
	//mX, mY - starting coordinates
	Ball(float mX, float mY)
	{
		shape.setPosition(mX, mY);
		shape.setRadius(ballRadius);
		shape.setFillColor(Color::Red);
		shape.setOrigin(ballRadius, ballRadius);
	}

	//Moving the ball by it's current velocity
	void update() { 
		if (not_moving && Keyboard::isKeyPressed(Keyboard::Key::Space)) {
			velocity.y = ballVelocity;
		}
		shape.move(velocity); 

		//if leaving the screen - change the direction of directional velocity
		if (left() <= 0) { velocity.x = ballVelocity; }
		else if (right() >= windowWidth) { velocity.x = -ballVelocity; }

		if (top() <= 0) { velocity.y = ballVelocity; }
		else if (bottom() >= windowHeight) { 

			if (Keyboard::isKeyPressed(Keyboard::Key::R) && lives>0) {
				shape.setPosition(windowWidth / 2, windowHeight / 2);
				velocity.x = 0;
				velocity.y = 0;
				not_moving = true;
				lives--;
			}
		}
	};

	void resetBall() {
		shape.setPosition(windowWidth / 2, windowHeight / 2);
	}

	//information aobut the ball
	float x() { return shape.getPosition().x; }
	float y() { return shape.getPosition().y; }
	float left() { return shape.getPosition().x - shape.getRadius(); }
	float right() { return x() + shape.getRadius(); }
	float top() { return y() - shape.getRadius(); }
	float bottom() { return y() + shape.getRadius(); }
};

struct Rectangle {
	RectangleShape shape;
	Vector2f velocity;

	//constructor
	Rectangle(float mX, float mY) {
		shape.setPosition(mX, mY);
		shape.setSize({ rectangleWidth, rectangleHeigth });
		shape.setFillColor(Color::Red);
		shape.setOrigin(rectangleWidth / 2.f, rectangleHeigth / 2.f);
	}
	void update() {
		shape.move(velocity);
		//steering with arrows
		//move only when arrow is pressed
		if (Keyboard::isKeyPressed(Keyboard::Key::Left) && left() > 0) velocity.x = -rectangleVelocity;
		else if (Keyboard::isKeyPressed(Keyboard::Key::Right) && right() < windowWidth) velocity.x = rectangleVelocity;
		else velocity.x = 0;
	}

	void resetRectangle() {
		shape.setPosition(windowWidth / 2, windowHeight - 50);
	}

	float x() { return shape.getPosition().x; }
	float y() { return shape.getPosition().y; }
	float left() { return x() - shape.getSize().x / 2.f; }
	float right() { return x() + shape.getSize().x / 2.f; }
	float top() { return y() - shape.getSize().y / 2.f; }
	float bottom() { return y() + shape.getSize().y / 2.f; }
};

// Let's have a class `Brick` for the bricks.
struct Brick
{
	RectangleShape shape;

	// This boolean value will be used to check
	// whether a brick has been hit or not.
	bool destroyed{ false };

	// The constructor is almost identical to the `Rectangle` one.
	Brick(float mX, float mY)
	{
		shape.setPosition(mX, mY);
		shape.setSize({ blockWidth, blockHeight });
		shape.setFillColor(Color::Yellow);
		shape.setOrigin(blockWidth / 2.f, blockHeight / 2.f);
	}

	float x() { return shape.getPosition().x; }
	float y() { return shape.getPosition().y; }
	float left() { return x() - shape.getSize().x / 2.f; }
	float right() { return x() + shape.getSize().x / 2.f; }
	float top() { return y() - shape.getSize().y / 2.f; }
	float bottom() { return y() + shape.getSize().y / 2.f; }
};

// Dealing with collisions
template <class T1, class T2>
bool isIntersecting(T1& mA, T2& mB)
{
	return mA.right() >= mB.left() && mA.left() <= mB.right() &&
		mA.bottom() >= mB.top() && mA.top() <= mB.bottom();
}

void testCollision(Rectangle& mRectangle, Ball& mBall)
{
	// If there's no intersection, get out of the function.
	if (!isIntersecting(mRectangle, mBall)) return;

	// Otherwise let's "push" the ball upwards.
	mBall.velocity.y = -ballVelocity;

	// And let's direct it dependently on the position where the
	// paddle was hit.
	if (mBall.x() < mRectangle.x())
		mBall.velocity.x = -ballVelocity;
	else
		mBall.velocity.x = ballVelocity;
}

void testCollision(Brick& mBrick, Ball& mBall)
{
	// If there's no intersection, get out of the function.
	if (!isIntersecting(mBrick, mBall)) return;

	// Otherwise, the brick has been hit!
	mBrick.destroyed = true;

	// Let's calculate how much the ball intersects the brick
	// in every direction.
	float overlapLeft{ mBall.right() - mBrick.left() };
	float overlapRight{ mBrick.right() - mBall.left() };
	float overlapTop{ mBall.bottom() - mBrick.top() };
	float overlapBottom{ mBrick.bottom() - mBall.top() };

	// If the magnitude of the left overlap is smaller than the
	// right one we can safely assume the ball hit the brick
	// from the left.
	bool ballFromLeft(abs(overlapLeft) < abs(overlapRight));

	// We can apply the same idea for top/bottom collisions.
	bool ballFromTop(abs(overlapTop) < abs(overlapBottom));

	// Let's store the minimum overlaps for the X and Y axes.
	float minOverlapX{ ballFromLeft ? overlapLeft : overlapRight };
	float minOverlapY{ ballFromTop ? overlapTop : overlapBottom };

	// If the magnitude of the X overlap is less than the magnitude
	// of the Y overlap, we can safely assume the ball hit the brick
	// horizontally - otherwise, the ball hit the brick vertically.

	// Then, upon our assumptions, we change either the X or Y velocity
	// of the ball, creating a "realistic" response for the collision.
	if (abs(minOverlapX) < abs(minOverlapY))
		mBall.velocity.x = ballFromLeft ? -ballVelocity : ballVelocity;
	else
		mBall.velocity.y = ballFromTop ? -ballVelocity : ballVelocity;
}

int main()
{
	if (!font.loadFromFile("../arial.ttf")) {}

	//Create the ball
	Ball ball{ windowWidth / 2, windowHeight / 2 };

	//Create the rectangle
	Rectangle rectangle{ windowWidth / 2, windowHeight -50};

	vector<Brick> bricks;


	for (int iX{ 0 }; iX < countBlocksX; ++iX)
		for (int iY{ 0 }; iY < countBlocksY; ++iY)
			bricks.emplace_back(
			(iX + 1) * (blockWidth + 3) + 22, (iY + 2) * (blockHeight + 3));

	// Creation of the game window.
	RenderWindow window{ { windowWidth, windowHeight }, "Arkanoid" };
	window.setFramerateLimit(60);

	Text atext;
	atext.setFont(font);
	atext.setCharacterSize(20);
	atext.setStyle(sf::Text::Bold);
	atext.setFillColor(sf::Color::White);
	atext.setPosition(0, 0);
	atext.setString("test");

	// Game loop.
	while (true)
	{
		// "Clear" the window from previously drawn graphics.
		window.clear(Color::Black);

		// If "Escape" is pressed, break out of the loop.
		if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) break;

		//updating the ball every loop iteration
		ball.update();
		rectangle.update();

		testCollision(rectangle, ball);

		// test collisions between the ball and EVERY brick.
		for (auto& brick : bricks) testCollision(brick, ball);

		// remove all `destroyed`
		bricks.erase(remove_if(begin(bricks), end(bricks),
			[](const Brick& mBrick)
		{
			return mBrick.destroyed;
		}),
			end(bricks));

		if (bricks.empty()) {
			/**
			TODO
			Display text
			**/
			ball.velocity.x = 0; //stop the ball
			ball.velocity.y = 0;
			if (Keyboard::isKeyPressed(Keyboard::Key::T)){
				ball.resetBall();
				rectangle.resetRectangle();
				for (int iX{ 0 }; iX < countBlocksX; ++iX)
					for (int iY{ 0 }; iY < countBlocksY; ++iY)
						bricks.emplace_back(
						(iX + 1) * (blockWidth + 3) + 22, (iY + 2) * (blockHeight + 3));
			}
		}

		if (lives == 0 && Keyboard::isKeyPressed(Keyboard::Key::T)) {
			ball.resetBall();
			rectangle.resetRectangle();
			for (int iX{ 0 }; iX < countBlocksX; ++iX)
				for (int iY{ 0 }; iY < countBlocksY; ++iY)
					bricks.emplace_back(
					(iX + 1) * (blockWidth + 3) + 22, (iY + 2) * (blockHeight + 3));
		}
		//Draw ball
		window.draw(ball.shape);
		window.draw(rectangle.shape);

		// Draw bricks
		for (auto& brick : bricks) window.draw(brick.shape);

		window.draw(atext);

		// Show the window contents.
		window.display();
	}

	return 0;
}