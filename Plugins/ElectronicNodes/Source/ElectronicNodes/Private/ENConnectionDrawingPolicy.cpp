/* Copyright (C) 2019 Hugo ATTAL - All Rights Reserved
 * This plugin is downloadable from the UE4 Marketplace
 */

#include "ENConnectionDrawingPolicy.h"
#include "Misc/App.h"
#include "BlueprintEditorSettings.h"
#include "Editor.h"
#include "AnimationGraphSchema.h"
#include "ENAnimGraphConnectionDrawingPolicy.h"
#include "ENPathDrawer.h"
#include "SGraphPanel.h"
#include "Framework/Application/SlateApplication.h"

FConnectionDrawingPolicy* FENConnectionDrawingPolicyFactory::CreateConnectionPolicy(const class UEdGraphSchema* Schema, int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const class FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const
{

	const UElectronicNodesSettings& ElectronicNodesSettings = *GetDefault<UElectronicNodesSettings>();
	if (!ElectronicNodesSettings.MasterActivate)
	{
		return nullptr;
	}

	if (Schema->IsA(UAnimationGraphSchema::StaticClass()))
	{
		if (ElectronicNodesSettings.ActivateOnAnimation)
		{
			return new FENAnimGraphConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements, InGraphObj);
		}
		return nullptr;
	}

	if (Schema->IsA(UMaterialGraphSchema::StaticClass()))
	{
		if (ElectronicNodesSettings.ActivateOnMaterial)
		{
			return new FENConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements, InGraphObj);
		}
		return nullptr;
	}

	if (Schema->IsA(UEdGraphSchema_K2::StaticClass()))
	{
		if (ElectronicNodesSettings.ActivateOnBlueprint)
		{
			return new FENConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements, InGraphObj);
		}
		return nullptr;
	}

	return nullptr;
}

void FENConnectionDrawingPolicy::DrawConnection(int32 layerId, const FVector2D& Start, const FVector2D& End, const FConnectionParams& params)
{
	if (params.AssociatedPin1 &&
		params.AssociatedPin1->GetOwningNode() &&
		params.AssociatedPin1->GetOwningNode()->IsSelected())
	{
		return;
	}

	bool RightPriority = ENIsRightPriotity(params);

	this->LayerId = layerId;
	this->Params = &params;
	ClosestDistanceSquared = MAX_FLT;

	FENPathDrawer PathDrawer(layerId, ZoomFactor, RightPriority, &params, &DrawElementsList, this);
	FVector2D StartDirection = (params.StartDirection == EEdGraphPinDirection::EGPD_Output) ? FVector2D(1.0f, 0.0f) : FVector2D(-1.0f, 0.0f);
	FVector2D EndDirection = (params.EndDirection == EEdGraphPinDirection::EGPD_Input) ? FVector2D(1.0f, 0.0f) : FVector2D(-1.0f, 0.0f);

	FVector2D new_Start = Start;
	FVector2D new_End = End;

	ENCorrectZoomDisplacement(new_Start, new_End);
	ENProcessRibbon(layerId, new_Start, StartDirection, new_End, EndDirection, params);

	const float Offset = ElectronicNodesSettings.HorizontalOffset * ZoomFactor;

	if (ElectronicNodesSettings.DisablePinOffset)
	{
		if (!((params.AssociatedPin1 != nullptr) && (params.AssociatedPin1->GetName() == "OutputPin")))
		{
			PathDrawer.DrawOffset(new_Start, StartDirection, Offset, false);
		}
		if (!((params.AssociatedPin2 != nullptr) && (params.AssociatedPin2->GetName() == "InputPin")))
		{
			PathDrawer.DrawOffset(new_End, EndDirection, Offset, true);
		}
	}
	else
	{
		PathDrawer.DrawOffset(new_Start, StartDirection, Offset, false);
		PathDrawer.DrawOffset(new_End, EndDirection, Offset, true);
	}

	EWireStyle WireStyle = ElectronicNodesSettings.WireStyle;

	if (ElectronicNodesSettings.OverwriteExecWireStyle)
	{
		if (((params.AssociatedPin1 != nullptr) && params.AssociatedPin1->PinType.PinCategory.ToString() == "exec") ||
			((params.AssociatedPin2 != nullptr) && params.AssociatedPin2->PinType.PinCategory.ToString() == "exec"))
		{
			if (ElectronicNodesSettings.WireStyleForExec != EWireStyle::Default)
			{
				WireStyle = ElectronicNodesSettings.WireStyleForExec;
			}
		}
	}

	switch (WireStyle)
	{
	case EWireStyle::Manhattan:
		PathDrawer.DrawManhattanWire(new_Start, StartDirection, new_End, EndDirection);
		break;
	case EWireStyle::Subway:
		PathDrawer.DrawSubwayWire(new_Start, StartDirection, new_End, EndDirection);
		break;
	default:
		PathDrawer.DrawDefaultWire(new_Start, StartDirection, new_End, EndDirection);
	}

	if (Settings->bTreatSplinesLikePins)
	{
		const float QueryDistanceTriggerThresholdSquared = FMath::Square(Settings->SplineHoverTolerance + params.WireThickness * 0.5f);
		const bool bCloseToSpline = ClosestDistanceSquared < QueryDistanceTriggerThresholdSquared;

		if (bCloseToSpline)
		{
			if (ClosestDistanceSquared < SplineOverlapResult.GetDistanceSquared())
			{
				const float SquaredDistToPin1 = (params.AssociatedPin1 != nullptr) ? (Start - ClosestPoint).SizeSquared() : FLT_MAX;
				const float SquaredDistToPin2 = (params.AssociatedPin2 != nullptr) ? (End - ClosestPoint).SizeSquared() : FLT_MAX;

				SplineOverlapResult = FGraphSplineOverlapResult(params.AssociatedPin1, params.AssociatedPin2, ClosestDistanceSquared, SquaredDistToPin1, SquaredDistToPin2);
			}
		}
	}
}

void FENConnectionDrawingPolicy::ENCorrectZoomDisplacement(FVector2D& Start, FVector2D& End)
{
	if (ElectronicNodesSettings.FixZoomDisplacement)
	{
		const float ZoomDisplacement = ZoomFactor * -19.0f + 8.0f;
		if (ZoomDisplacement > 0)
		{
			Start.X += ZoomDisplacement / 2.0f;
			End.X -= ZoomDisplacement / 2.0f;
		}
	}
}

void FENConnectionDrawingPolicy::ENProcessRibbon(int32 layerId, FVector2D& Start, FVector2D& StartDirection, FVector2D& End, FVector2D& EndDirection, const FConnectionParams& params)
{
	int32 DepthOffsetX = 0;
	int32 DepthOffsetY = 0;
	const float SplineSize = (End - Start).Size();

	if (ElectronicNodesSettings.ActivateRibbon)
	{
		for (ENRibbonConnection RibbonConnection : RibbonConnections)
		{
			if (RibbonConnection.Horizontal)
			{
				if (FMath::Abs(Start.Y - RibbonConnection.Main) < ElectronicNodesSettings.RibbonOffset)
				{
					const float CurrentMax = FMath::Max(Start.X, End.X);
					const float CurrentMin = FMath::Min(Start.X, End.X);
					const float RibbonMax = FMath::Max(RibbonConnection.Start, RibbonConnection.End);
					const float RibbonMin = FMath::Min(RibbonConnection.Start, RibbonConnection.End);

					if (FMath::IsNearlyEqual(RibbonMin, CurrentMin, KINDA_SMALL_NUMBER) ||
						FMath::IsNearlyEqual(RibbonMax, CurrentMin, KINDA_SMALL_NUMBER) ||
						FMath::IsNearlyEqual(RibbonMin, CurrentMax, KINDA_SMALL_NUMBER) ||
						FMath::IsNearlyEqual(RibbonMax, CurrentMax, KINDA_SMALL_NUMBER))
					{
						continue;
					}

					if (FMath::Min(CurrentMax, RibbonMax) > FMath::Max(CurrentMin, RibbonMin) - 1.0f)
					{
						if (End.Y - RibbonConnection.Sub > 0)
						{
							DepthOffsetY = FMath::Max(DepthOffsetY, FMath::Max(1, RibbonConnection.Depth + 1));
						}
						else
						{
							DepthOffsetY = FMath::Min(DepthOffsetY, FMath::Min(-1, RibbonConnection.Depth - 1));
						}
					}
				}
			}
		}
		for (ENRibbonConnection RibbonConnection : RibbonConnections)
		{
			if (!RibbonConnection.Horizontal)
			{
				if (FMath::Abs(End.X - RibbonConnection.Main) < ElectronicNodesSettings.RibbonOffset)
				{
					const float CurrentMax = FMath::Max(Start.Y, End.Y);
					const float CurrentMin = FMath::Min(Start.Y, End.Y);
					const float RibbonMax = FMath::Max(RibbonConnection.Start, RibbonConnection.End);
					const float RibbonMin = FMath::Min(RibbonConnection.Start, RibbonConnection.End);

					if (FMath::IsNearlyEqual(RibbonMin, CurrentMin, KINDA_SMALL_NUMBER) ||
						FMath::IsNearlyEqual(RibbonMax, CurrentMin, KINDA_SMALL_NUMBER) ||
						FMath::IsNearlyEqual(RibbonMin, CurrentMax, KINDA_SMALL_NUMBER) ||
						FMath::IsNearlyEqual(RibbonMax, CurrentMax, KINDA_SMALL_NUMBER))
					{
						continue;
					}

					if (FMath::Min(CurrentMax, RibbonMax) > FMath::Max(CurrentMin, RibbonMin) - 1.0f)
					{
						if ((Start.Y - RibbonConnection.Start) * FMath::Sign(End.Y - Start.Y) > 0)
						{
							DepthOffsetX = FMath::Max(DepthOffsetX, FMath::Max(1, RibbonConnection.Depth + 1));
						}
						else
						{
							DepthOffsetX = FMath::Min(DepthOffsetX, FMath::Min(-1, RibbonConnection.Depth - 1));
						}

						if (DepthOffsetY != 0)
						{
							DepthOffsetX = FMath::Sign(End.Y - Start.Y) * DepthOffsetY;
						}
					}
				}
			}
		}

		RibbonConnections.Add(ENRibbonConnection(Start.Y, End.Y, true, Start.X, End.X, DepthOffsetY));
		RibbonConnections.Add(ENRibbonConnection(End.X, Start.X, false, Start.Y, End.Y, DepthOffsetX));

		FVector2D StartKey(FMath::FloorToInt(Start.X), FMath::FloorToInt(Start.Y));
		FVector2D EndKey(FMath::FloorToInt(End.X), FMath::FloorToInt(End.Y));

		FENPathDrawer PathDrawer(layerId, ZoomFactor, true, &params, &DrawElementsList, this);

		if (DepthOffsetY != 0)
		{
			FVector2D newStart = Start;
			newStart.X += ElectronicNodesSettings.RibbonMergeOffset * ZoomFactor * StartDirection.X;
			newStart.Y += (int32)ElectronicNodesSettings.RibbonOffset * DepthOffsetY * ZoomFactor;

			PathDrawer.DrawManhattanWire(Start, StartDirection, newStart, StartDirection);

			Start = newStart;
		}

		if (DepthOffsetX != 0)
		{
			FVector2D newEnd = End;
			newEnd.X -= (int32)ElectronicNodesSettings.RibbonOffset * DepthOffsetX * ZoomFactor * EndDirection.X;

			if (DepthOffsetX * EndDirection.X > 0)
			{
				PathDrawer.DrawManhattanWire(newEnd, EndDirection, End, EndDirection);
			}

			End = newEnd;
		}
	}
}

bool FENConnectionDrawingPolicy::ENIsRightPriotity(const FConnectionParams& params)
{
	bool RightPriority = (ElectronicNodesSettings.WireAlignment == EWireAlignment::Right);
	EWirePriority WirePriority = ElectronicNodesSettings.WirePriority;

	if (ElectronicNodesSettings.OverwriteExecWireStyle)
	{
		if (((params.AssociatedPin1 != nullptr) && params.AssociatedPin1->PinType.PinCategory.ToString() == "exec") ||
			((params.AssociatedPin2 != nullptr) && params.AssociatedPin2->PinType.PinCategory.ToString() == "exec"))
		{
			RightPriority = (ElectronicNodesSettings.WireAlignmentForExec == EWireAlignment::Right);
			WirePriority = ElectronicNodesSettings.WirePriorityForExec;
		}
	}

	if (WirePriority != EWirePriority::None)
	{
		if ((params.AssociatedPin1 != nullptr) && (params.AssociatedPin2 != nullptr))
		{
			bool IsOutputPin = (params.AssociatedPin1->GetName() == "OutputPin");
			bool IsInputPin = (params.AssociatedPin2->GetName() == "InputPin");
			if (IsOutputPin ^ IsInputPin)
			{
				switch (WirePriority)
				{
				case EWirePriority::Node:
					RightPriority = IsOutputPin;
					break;
				case EWirePriority::Pin:
					RightPriority = IsInputPin;
					break;
				}
			}
		}
	}

	return RightPriority;
}

int32 FENConnectionDrawingPolicy::ENGetZoomLevel()
{
	const float ZoomLevels[] = { 2.0f, 1.875f, 1.75f, 1.675f, 1.5f, 1.375f, 1.25f, 1.0f, 0.875f, 0.75f, 0.675f, 0.5f, 0.375f, 0.25f, 0.225f, 0.2f, 0.175f, 0.15f, 0.125f, 0.1f };

	for (int32 i = 0; i < 20; i++)
	{
		if (ZoomFactor > ZoomLevels[i] - KINDA_SMALL_NUMBER)
		{
			return 7 - i;
		}
	}

	return -12;
}

TSharedPtr<SGraphPanel> FENConnectionDrawingPolicy::GetGraphPanel()
{
	FSlateApplication& slateApplication = FSlateApplication::Get();
	TSharedPtr<SWidget> widget = slateApplication.GetUserFocusedWidget(0);
	if (widget.IsValid() && widget->GetTypeAsString() == "SGraphPanel")
	{
		return StaticCastSharedPtr<SGraphPanel>(widget);
	}
	return nullptr;
}

void FENConnectionDrawingPolicy::BuildRelatedNodes(UEdGraphNode* Node, TArray<UEdGraphNode*>& RelatedNodes, bool InputCheck = true, bool OutputCheck = true)
{
	if (RelatedNodes.Find(Node) != INDEX_NONE && (!InputCheck || !OutputCheck))
	{
		return;
	}
	RelatedNodes.Push(Node);

	for (auto Pin : Node->Pins)
	{
		if (InputCheck && Pin->Direction == EEdGraphPinDirection::EGPD_Input)
		{
			for (auto LinkedPin : Pin->LinkedTo)
			{
				UEdGraphNode* LinkedNode = LinkedPin->GetOwningNode();
				if (ElectronicNodesSettings.SelectionRule == ESelectionRule::Far || LinkedNode->GetName().StartsWith("K2Node_Knot_"))
				{
					BuildRelatedNodes(LinkedNode, RelatedNodes, true, false);
				}
				else
				{
					RelatedNodes.Push(LinkedNode);
				}
			}
		}

		if (OutputCheck && Pin->Direction == EEdGraphPinDirection::EGPD_Output)
		{
			for (auto LinkedPin : Pin->LinkedTo)
			{
				UEdGraphNode* LinkedNode = LinkedPin->GetOwningNode();
				if (ElectronicNodesSettings.SelectionRule == ESelectionRule::Far || LinkedNode->GetName().StartsWith("K2Node_Knot_"))
				{
					BuildRelatedNodes(LinkedNode, RelatedNodes, false, true);
				}
				else
				{
					RelatedNodes.Push(LinkedNode);
				}
			}
		}
	}
}

void FENConnectionDrawingPolicy::ENDrawBubbles(const FVector2D& Start, const FVector2D& StartTangent, const FVector2D& End, const FVector2D& EndTangent)
{
	bool ENDrawBubbles = ElectronicNodesSettings.ForceDrawBubbles && (ElectronicNodesSettings.BubbleZoomThreshold <= ENGetZoomLevel());
	if (Params->bDrawBubbles || ENDrawBubbles)
	{
		bool LinkedBubbles = true;

		if (!Params->bDrawBubbles)
		{
			LinkedBubbles = false;

			if (ElectronicNodesSettings.BubbleDisplayRule == EBubbleDisplayRule::DisplayOnSelection ||
				ElectronicNodesSettings.BubbleDisplayRule == EBubbleDisplayRule::MoveOnSelection)
			{
				TSharedPtr<SGraphPanel> GraphPanel = this->GetGraphPanel();
				if (GraphPanel.IsValid())
				{
					if (Params->AssociatedPin1 != nullptr && Params->AssociatedPin2 != nullptr)
					{
						for (auto SelectedNode : GraphPanel->SelectionManager.SelectedNodes) {
							TArray<UEdGraphNode*> RelatedNodes;
							UEdGraphNode* SelectedGraphNode = StaticCast<UEdGraphNode*>(SelectedNode);
							this->BuildRelatedNodes(SelectedGraphNode, RelatedNodes);

							if (RelatedNodes.Find(Params->AssociatedPin1->GetOwningNode()) != INDEX_NONE && RelatedNodes.Find(Params->AssociatedPin2->GetOwningNode()) != INDEX_NONE)
							{
								LinkedBubbles = true;
							}
						}
					}
				}
			}
		}

		if (!LinkedBubbles && ElectronicNodesSettings.BubbleDisplayRule == EBubbleDisplayRule::DisplayOnSelection)
		{
			return;
		}

		FInterpCurve<float> SplineReparamTable;
		const float SplineLength = (Start - End).Size();
		int32 NumBubbles = FMath::CeilToInt(SplineLength / (ElectronicNodesSettings.BubbleSpace * ZoomFactor));
		NumBubbles = FMath::Min(NumBubbles, 1000);

		float BubbleSpeed = ElectronicNodesSettings.BubbleSpeed;
		if (!LinkedBubbles && ElectronicNodesSettings.BubbleDisplayRule == EBubbleDisplayRule::MoveOnSelection)
		{
			BubbleSpeed = 0.0f;
		}
		FVector2D BubbleSize = BubbleImage->ImageSize * ZoomFactor * 0.1f * ElectronicNodesSettings.BubbleSize * FMath::Sqrt(Params->WireThickness);
		if (Params->bDrawBubbles)
		{
			BubbleSize *= 1.25f;
		}
		const float Time = (FPlatformTime::Seconds() - GStartTime);

		float AlphaOffset = FMath::Frac(Time * BubbleSpeed);

		for (int32 i = 0; i < NumBubbles; ++i)
		{
			const float Alpha = (AlphaOffset + i) / NumBubbles;
			FVector2D BubblePos;
			if (StartTangent != FVector2D::ZeroVector && EndTangent != FVector2D::ZeroVector)
			{
				if ((StartTangent != EndTangent) && ((StartTangent * EndTangent) == FVector2D::ZeroVector))
				{
					BubblePos = Start + StartTangent * FMath::Sin(Alpha * PI / 2.0f) + EndTangent * (1.0f - FMath::Cos(Alpha * PI / 2.0f));
				}
				else
				{
					BubblePos = FMath::CubicInterp(Start, StartTangent, End, EndTangent, Alpha);
				}
			}
			else
			{
				BubblePos = FMath::Lerp(Start, End, Alpha);
			}
			BubblePos -= (BubbleSize * 0.5f);

			FSlateDrawElement::MakeBox(
				DrawElementsList,
				LayerId,
				FPaintGeometry(BubblePos, BubbleSize, ZoomFactor),
				BubbleImage,
				ESlateDrawEffect::None,
				Params->WireColor
			);
		}
	}
}

void FENConnectionDrawingPolicy::ENDrawArrow(const FVector2D& Start, const FVector2D& End)
{
	if (MidpointImage != nullptr && (Start - End).Size() > 4 * MinXOffset)
	{
		const FVector2D MidpointDrawPos = (Start + End) / 2.0f - MidpointRadius * 0.75f;
		const FVector2D SlopeUnnormalized = (End - Start);
		const float AngleInRadians = FMath::Atan2(SlopeUnnormalized.Y, SlopeUnnormalized.X);

		FSlateDrawElement::MakeRotatedBox(DrawElementsList, LayerId, FPaintGeometry(MidpointDrawPos, MidpointImage->ImageSize * ZoomFactor * 0.75f, ZoomFactor * 0.75f),
			MidpointImage, ESlateDrawEffect::None, AngleInRadians, TOptional<FVector2D>(), FSlateDrawElement::RelativeToElement, Params->WireColor);
	}
}

void FENConnectionDrawingPolicy::DrawDebugPoint(const FVector2D& Position, FLinearColor Color)
{
	const FVector2D BubbleSize = BubbleImage->ImageSize * ZoomFactor * 0.1f * ElectronicNodesSettings.BubbleSize * FMath::Sqrt(Params->WireThickness);
	const FVector2D BubblePos = Position - (BubbleSize * 0.5f);

	FSlateDrawElement::MakeBox(
		DrawElementsList,
		LayerId,
		FPaintGeometry(BubblePos, BubbleSize, ZoomFactor),
		BubbleImage,
		ESlateDrawEffect::None,
		Color
	);
}

void FENConnectionDrawingPolicy::ENComputeClosestPoint(const FVector2D& Start, const FVector2D& End)
{
	const FVector2D TemporaryPoint = FMath::ClosestPointOnSegment2D(LocalMousePosition, Start, End);
	const float TemporaryDistance = (LocalMousePosition - TemporaryPoint).SizeSquared();

	if (TemporaryDistance < ClosestDistanceSquared)
	{
		ClosestDistanceSquared = TemporaryDistance;
		ClosestPoint = TemporaryPoint;
	}
}
